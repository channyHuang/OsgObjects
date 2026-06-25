#include "ImguiMainPage.h"

#include "generator/terrainModification.h"
#include "physics/physicsManager.h"
#include "commonDef.h"

GLuint textureID;

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

ImguiMainPage::ImguiMainPage() {}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
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

void ImguiMainPage::drawUi() {
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
            TerrainModification::getInstance()->activate = false;

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
            if (ImGui::Button("test sdf")) {
                //Generator_Recon::getInstance()->loadFile(cFileName);
            }

            ImGui::ProgressBar(TerrainGenerator_Roblox::getInstance()->progress);

            if (ImGui::Button("clear")) {
                OsgManager::getInstance()->clear();
                // VoxelMap::getInstance()->for_all_blocks([=](VoxelBlock* block) {
                //     VoxelMap::getInstance()->remove_block(block->position);
                // });
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
