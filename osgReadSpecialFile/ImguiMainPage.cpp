#include "ImguiMainPage.h"

#include "osgManager.h"
#include "nativefiledialog/nfd.h"
GLuint textureID;

#ifdef WiN32
#include <windows.h>

std::wstring wstr(std::string temp) {
    int size = MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), &result[0], size);
    return result;
}

std::wstring utf8_to_utf16(const std::string& utf8) {
    std::wstring utf16;
    if (!utf8.empty()) {
        int size = MultiByteToWideChar(CP_UTF8, 0, &utf8[0], (int)utf8.size(), NULL, 0);
        utf16.resize(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &utf8[0], (int)utf8.size(), &utf16[0], size);
    }
    return utf16;
}
#endif

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);

    m_pPicker = new PickHandler();
    m_pPicker->setCallback(pickCbFunc, nullptr);
    viewer.addEventHandler(m_pPicker);

    cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
    if (cFileName != nullptr) {
        delete[]cFileName;
    }
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("post raw volume");
    
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {}
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog("bin"/*"obj,ply,xyz,csv"*/, nullptr, &cFileName);
        if (result == NFD_OKAY) {
        }
    }

    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    if (ImGui::Button("Reset Scene")) {
        m_pCameraHandler->reset();
    }

    ImGui::Checkbox("Back Scene To World Center", &m_pCameraHandler->m_bBack2WorldCenter);
    if (m_pCameraHandler->m_bBack2WorldCenter) {
        m_pCameraHandler->back2WorldCenter();
        m_pCameraHandler->m_bBack2WorldCenter = false;
    }

    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Load raw volume of Instant-ngp"))
        {
            ImGui::SliderInt("cascade", &cascade, 0, 7);
            ImGui::SliderFloat("thresMin", &thresMin, -1, 1024);
            ImGui::SliderFloat("thresMax", &thresMax, -1, 1024);

            if (ImGui::Button("load raw volume")) {
                load("E:/tmp_res/lab_sub/volume_raw");
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Load Special Model")) {

            if (ImGui::Button("load special ply")) {
                OsgManager::getInstance()->loadSpecialPly();
            }
            if (ImGui::Button("load special ply and save to osgb")) {
                OsgManager::getInstance()->transfer();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ImguiMainPage::load(std::string sFile) {
#ifdef WIN32
    uint32_t d = 256, h = 256, w = 256;
    std::ofstream ofs("output.obj");

    int maxn = thresMax;
    if (maxn < 0) maxn = 1e+010;
    std::cout << thresMin << " " << thresMax << std::endl;
    FILE* fin = _wfopen(wstr(sFile + "/256x256x256_" + std::to_string(cascade) + ".bin").c_str(), utf8_to_utf16("rb").c_str());
    float move = d / 2.f;
    float value[4] = { 0 };
    for (int z = 0; z < d; ++z) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                fread(value, sizeof(float), 4, fin);

                //if (cascade && x>=d/4+border && y>=d/4+border && z>=d/4+border && x<d-d/4-border && y<d-d/4-border && z<d-d/4-border) continue;
                int offset = (1 << cascade);
                if (value[3] >= thresMin && value[3] <= maxn)
                {
                    ofs << "v " << (x - move) * offset << " " << (y - move) * offset << " " << (z - move) * offset << " " << value[0] << " " << value[1] << " " << value[2] << " " << value[3] << " " << std::endl;
                }
            }
        }
    }

    fclose(fin);
    ofs.close();
#endif
    std::cout << "done " << std::endl;
}
