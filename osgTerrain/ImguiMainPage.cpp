#include "ImguiMainPage.h"

#include "generator/terraingenerator_roblox.h"
#include "generator/generator_recon.h"
#include "collaborative/trainTestCol.h"

#include "generator/terrainModification.h"
#include "physics/physicsManager.h"

GLuint textureID;
TerrainGenerator_Roblox::BiomesParam stBiomeParams;

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer) {
    pviewer = &viewer;
    OsgManager::getInstance()->setViewer(viewer);

    notify.connect(TerrainManager::getInstance(), &TerrainManager::_notification);
    TerrainManager::getInstance()->sigGenerateMeshSuc.connect(OsgManager::getInstance(), &OsgManager::updateTerrain);
#ifdef PHYSICS_ON
    PhysicsManager::getInstance()->updateActor.connect(OsgManager::getInstance(), &OsgManager::updateDynamicActors);
#endif
    notify(Notification_Enter);

    startTime = clock();
    frameTime = clock();

    cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);
}

ImguiMainPage::~ImguiMainPage() {
    notify(Notification_Exit);

    pviewer = nullptr;
}

static void genTerrain(TerrainGenerator_Roblox::BiomesParam& param) {
    TerrainGenerator_Roblox::getInstance()->generateTerrainByBiomes(OsgManager::getInstance()->brush, stBiomeParams);
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("3d reconstruction");
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog(""/*"obj,ply,xyz,csv"*/, nullptr, &cFileName);
        if (result == NFD_OKAY) {
        }
    }
    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    if (ImGui::Button("reset scene")) {
        OsgManager::getInstance()->clear();
    }
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Collaborative"))
        {
            if (ImGui::Button("gen data")) {
                TrainTestCol trainer;
                trainer.colliborate();
            }
            ImGui::Checkbox("run collaborative", &bCollaborative);
            if (bCollaborative) {
                //if ((clock() - frameTime) / CLOCKS_PER_SEC > 0.5) {
                    bool valid = OsgManager::getInstance()->Collaborate("mappo.txt");
                    if (!valid) {
                        bCollaborative = false;
                    }
                    frameTime = clock();
                //}
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Generate"))
        {
            TerrainModification::getInstance()->activate = false;

            ImGui::SeparatorText("size");
            ImGui::SliderInt("x##slider", &TerrainGenerator_Roblox::getInstance()->_range.vSize.x, 4, 1024);
            ImGui::SliderInt("y", &TerrainGenerator_Roblox::getInstance()->_range.vSize.y, 4, 1024);
            ImGui::SliderInt("z", &TerrainGenerator_Roblox::getInstance()->_range.vSize.z, 4, 1024);

            ImGui::SeparatorText("position");
            ImGui::SliderFloat("center x", &vCenter.x, -1024.f, 1024.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("center y", &vCenter.y, -1024.f, 1024.f);
            ImGui::SliderFloat("center z", &vCenter.z, -1024.f, 1024.f);

            ImGui::SeparatorText("biomes");
            ImGui::Checkbox("use biomes", &stBiomeParams.use_biomes);
            ImGui::Checkbox("generate caves", &stBiomeParams.generate_caves);
            ImGui::Separator();
            for (int i = 0; i < TerrainGenerator_Roblox::TerrainBiomes::TerrainBiomesCount; ++i) {
                if (ImGui::Checkbox(ReconSpace::vBiomeStr[i].c_str(), &ReconSpace::vBiomeCheck[i])) {
                    if (ReconSpace::vBiomeCheck[i]) stBiomeParams.biomes_be_checked |= (1 << i);
                    else stBiomeParams.biomes_be_checked ^= (1 << i);
                }
            }

            if (ImGui::Button("create")) {
                TerrainGenerator_Roblox::getInstance()->setRange(vCenter, TerrainGenerator_Roblox::getInstance()->_range.vSize);
                
                //std::thread genThread(genTerrain, std::ref(stBiomeParams));
                //genThread.detach();
                
                TerrainGenerator_Roblox::getInstance()->generateTerrainByBiomes(OsgManager::getInstance()->brush, stBiomeParams);
            }
            if (ImGui::Button("test sdf")) {
                //Generator_Recon::getInstance()->loadFile(cFileName);
            }

            ImGui::ProgressBar(TerrainGenerator_Roblox::getInstance()->progress);

            if (ImGui::Button("clear")) {
                OsgManager::getInstance()->clear();
                VoxelMap::getInstance()->for_all_blocks([=](VoxelBlock* block) {
                    VoxelMap::getInstance()->remove_block(block->position);
                });
            }

            if (ImGui::Button("switch wireFrame")) {
                OsgManager::getInstance()->switchScene();
            }

            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Physics Collision"))
        {
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

    if ((clock() - startTime) / CLOCKS_PER_SEC > 5) {
        startTime = clock();
        notify(Notification_Process);
    }
    
#ifdef PHYSICS_ON
    PhysicsManager::getInstance()->stepPhysics(true);
#endif
}
