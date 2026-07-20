#include "ImguiMainPage.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>

#include "generator/terrainModification.h"
#include "physics/physicsManager.h"
#include "commonDef.h"

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

GLuint LoadTextureFromFile(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4); // 强制 RGBA
    if (!data) {
        std::cout << "load texture failed: " << filename << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    return texture;
}

ImguiMainPage::ImguiMainPage() {}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandlerTerrain> pCameraHandler) {
    pViewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);
    // cTexturePath = new char[nMaxFileNameLength];
    // memset(cTexturePath, 0, nMaxFileNameLength);

    m_pPicker = new PickHandler();
    m_pPicker->setCallback(pickCbFunc, nullptr);
    viewer.addEventHandler(m_pPicker);

    notify.connect(TerrainManager::getInstance(), &TerrainManager::_notification);
    TerrainManager::getInstance()->sigGenerateMeshSuc.connect(OsgManager::getInstance(), &OsgManager::updateTerrain);
    pCameraHandler->sigPick.connect(OsgManager::getInstance(), &OsgManager::pick);

#ifdef PHYSICS_ON
    PhysicsManager::getInstance()->updateActor.connect(OsgManager::getInstance(), &OsgManager::updateDynamicActors);
#endif
    notify(Notification_Enter);

    stLastAutoUpdateTime = std::chrono::steady_clock::now();

    pFileName = new char[nMaxFileNameLength];
    memset(pFileName, 0, nMaxFileNameLength);

}

ImguiMainPage::~ImguiMainPage() {
    notify(Notification_Exit);
    pViewer = nullptr;
}

// 加载图片并生成纹理，返回纹理 ID（失败返回 0）


bool ImageRadioButton(const char* sId, GLuint textureID, ImVec2 size, int* v, int v_button) {
    bool bSelected = (*v == v_button);
    ImTextureID texID = (ImTextureID)(intptr_t)textureID;
    bool bClicked = ImGui::ImageButton(sId, texID, size);
    if (bClicked)
        *v = v_button;
    
    if (bSelected) {
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255,255,0,255), 0.0f, 0, 2.0f);
    }
    return bClicked;
}

void ImguiMainPage::drawUi() {
    if (m_bFirstLoad) {
        m_bFirstLoad = false;
        std::string sTexturePath = "../data/texture";
        std::vector<std::filesystem::path> sMaterialFiles;
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::absolute(sTexturePath))) {
            if (entry.is_regular_file()) {
                std::string sFileName = entry.path().filename().string();
                if (!sFileName.starts_with("material-")) continue;
                sMaterialFiles.emplace_back(entry.path());
            }
        }
        std::sort(sMaterialFiles.begin(), sMaterialFiles.end());
        
        for (auto sFileName : sMaterialFiles) {
            GLuint texID = LoadTextureFromFile(sFileName.c_str());
            std::string sTexID = sFileName.stem().string().substr(9);
            mapTextureID.insert(std::make_pair(sTexID, texID));
            // if (sSelectedTex.empty()) sSelectedTex = sTexID;
        }
    }
    ImGui::Begin("Terrain");

    if (ImGui::InputTextWithHint("Open terrain", "<.obj .ply .xyz>", pFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {}
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog(""/*"obj,ply,xyz,csv"*/, nullptr, &pFileName);
        if (result == NFD_OKAY) {
        }
    }

    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    if (ImGui::Button("reset to origin position")) {
        m_pCameraHandler->reset();
    }

    if (ImGui::BeginTabBar("Terrain Pannal", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Generator")) {
            TerrainModification::getInstance()->m_bActivate = false;

            ImGui::SeparatorText("size");
            ImGui::SliderInt("size x", &TerrainGenerator_Roblox::getInstance()->_range.vSize.x, 4, 1024);
            ImGui::SliderInt("size y", &TerrainGenerator_Roblox::getInstance()->_range.vSize.y, 4, 1024);
            ImGui::SliderInt("size z", &TerrainGenerator_Roblox::getInstance()->_range.vSize.z, 4, 1024);

            ImGui::SeparatorText("position");
            ImGui::SliderFloat("center x", &vCenter.x, -1024.f, 1024.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("center y", &vCenter.y, -1024.f, 1024.f);
            ImGui::SliderFloat("center z", &vCenter.z, -1024.f, 1024.f);

            ImGui::SeparatorText("Profile");
            ImGui::Checkbox("generate caves", &m_stBiomeParams.generate_caves);
            
            ImGui::SeparatorText("biomes");
            ImGui::Checkbox("use biomes", &m_stBiomeParams.use_biomes);
            if (m_stBiomeParams.use_biomes) {
                ImGui::Separator();
                for (int i = 0; i < TerrainGenerator_Roblox::TerrainBiomes::TerrainBiomesCount; ++i) {
                    if (ImGui::Checkbox(ReconSpace::vBiomeStr[i].c_str(), &ReconSpace::vBiomeCheck[i])) {
                        if (ReconSpace::vBiomeCheck[i]) m_stBiomeParams.biomes_be_checked |= (1 << i);
                        else m_stBiomeParams.biomes_be_checked ^= (1 << i);
                    }
                }
            }

            ImGui::SeparatorText("operator");
            if (ImGui::Button("create")) {
                TerrainGenerator_Roblox::getInstance()->setRange(vCenter, TerrainGenerator_Roblox::getInstance()->_range.vSize);
                TerrainGenerator_Roblox::getInstance()->generateTerrainByBiomes(OsgManager::getInstance()->m_pBrush, m_stBiomeParams);
            }
            ImGui::SameLine();
            ImGui::ProgressBar(TerrainGenerator_Roblox::getInstance()->progress);

            if (ImGui::Button("wireframe")) {
                OsgManager::getInstance()->onOffWireframe();
            }

            if (ImGui::Button("clear")) {
                OsgManager::getInstance()->clear();
                // VoxelMap::getInstance()->for_all_blocks([=](VoxelBlock* block) {
                //     VoxelMap::getInstance()->remove_block(block->position);
                // });
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Edit")) {
            TerrainModification::getInstance()->m_bActivate = true;

            ImGui::SeparatorText("Type");
            ImGui::RadioButton("Sphere", &TerrainModification::getInstance()->m_pTerrainBrush->m_eBrushType, 0);
            ImGui::RadioButton("Square", &TerrainModification::getInstance()->m_pTerrainBrush->m_eBrushType, 1);

            ImGui::SeparatorText("Size");
            ImGui::SliderFloat("brush size", &TerrainModification::getInstance()->m_pTerrainBrush->m_fSize, 1, 128, "%.1f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::SeparatorText("Optimize");
            ImGui::RadioButton("Add", &TerrainModification::getInstance()->m_eModifyType, 0);
            ImGui::RadioButton("Reduce", &TerrainModification::getInstance()->m_eModifyType, 1);
            ImGui::RadioButton("Flat", &TerrainModification::getInstance()->m_eModifyType, 2);
            ImGui::RadioButton("Replace", &TerrainModification::getInstance()->m_eModifyType, 3);

            ImGui::SeparatorText("Material");
            size_t i = 0;
            for (auto itr = mapTextureID.begin(); itr != mapTextureID.end(); itr++) {
                ImGui::PushID(itr->first.c_str());
                ImTextureID texID = (ImTextureID)(intptr_t)(itr->second);
                if ( ImGui::ImageButton(itr->first.c_str(), texID, ImVec2(32, 32)) ) {
                    sSelectedTex = itr->first;
                }
                if (sSelectedTex == itr->first) {
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImVec2 rectMin = ImGui::GetItemRectMin();
                    ImVec2 rectMax = ImGui::GetItemRectMax();
                    drawList->AddRect(rectMin, rectMax, IM_COL32(255, 0, 0, 255), 0.0f, 0, 3.0f);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", itr->first.c_str());
                }
                i++;
                if (i & 3) {
                    ImGui::SameLine();
                }
                ImGui::PopID();
            }

            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Physics Collision")) {
            ImGui::SliderInt("random balls", &nNumBalls, 1, 10);
            if (ImGui::Button("show collision")) {
                for (int i = 0; i < nNumBalls; ++i) {
                    osg::Vec3 pos(rand() * 1.f / RAND_MAX * 100 - 50, 100, rand() * 1.f / RAND_MAX * 100 - 50);
                    OsgManager::getInstance()->createSpheres(pos);
                }
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    // auto update
    auto now = std::chrono::steady_clock::now();
    if (now - stLastAutoUpdateTime > std::chrono::seconds(1)) {
        stLastAutoUpdateTime = now;
        notify(Notification_Process);
    }
    
#ifdef PHYSICS_ON
    PhysicsManager::getInstance()->stepPhysics(true);
#endif
}
