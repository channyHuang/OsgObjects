#include "ImguiMainPage.h"

#include "osgManager.h"
#include "commonOsg/commonOsg.h"
#include "nativefiledialog/nfd.h"
GLuint textureID;

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

ImguiMainPage::ImguiMainPage() {}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);
    cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);

    m_pPicker = new PickHandler();
    m_pPicker->setCallback(pickCbFunc, nullptr);
    viewer.addEventHandler(m_pPicker);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
    if (cFileName != nullptr) {
        delete[]cFileName;
    }
}

GLuint createTextureByData(const unsigned char* data, int width, int height) {
    GLuint textureID;
    const int length = width * height * 4;
    GLubyte* pixels = new GLubyte[length];
    memcpy(pixels, data, length * sizeof(char));

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    free(pixels);
    return textureID;
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("title");
    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog("obj,ply,xyz,csv", nullptr, &cFileName);
        if (result == NFD_OKAY) {

        }
    }
    if (ImGui::Button("Reset Scene")) {
        m_pCameraHandler->reset();
    }
    ImGui::Checkbox("Picker valid?", &m_pPicker->m_bCheckHit);
    ImGui::Checkbox("Back Scene To World Center", &m_pCameraHandler->m_bBack2WorldCenter);
    if (m_pCameraHandler->m_bBack2WorldCenter) {
        m_pCameraHandler->back2WorldCenter();
        m_pCameraHandler->m_bBack2WorldCenter = false;
    }
    ImGui::SliderFloat("step", &m_pCameraHandler->m_fStepScale, 0.01f, 3.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

    if (ImGui::Button("clear pick")) {
        OsgManager::getInstance()->clearPick();
    }

    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("tab name"))
        {
            // image button
            if (ImGui::Button("generate image data")) {
                // image
                std::vector<uint8_t> imgData;
                imgData.resize(100 * 100 * 4);
                memset(imgData.data(), 0, imgData.size());
                for (int i = 0; i < 100; ++i) {
                    for (int j = 0; j < 100; ++j) {
                        uint8_t* pixel = &imgData[(i + j * 100) * 4];
                        pixel[0] = rand() % 255;
                        pixel[1] = rand() % 255;
                        pixel[2] = rand() % 255;
                        pixel[3] = 255;
                    }
                }

                textureID = createTextureByData(imgData.data(), 100, 100);
            }

            ImGui::Checkbox("show image", &bShowImage);
            if (bShowImage) {
                ImTextureID my_tex_id = (GLuint*)textureID;
                if (ImGui::ImageTextButton("text around image", my_tex_id, ImVec4(1.f, 0.f, 0.f, 1.f), 0, ImVec2(100, 100))) {
                    std::cout << "ImageTextButton clicked" << std::endl;
                }

                if (ImGui::ImageButton("image button", my_tex_id, ImVec2(100, 100))) {
                    // do something
                }

                float my_tex_w = 100;
                float my_tex_h = 100;
                ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h));
            }

            // color button
            for (int i = 0; i < vMenuStr.size(); ++i) {
                if (i != 0) ImGui::SameLine();
                if (ImGui::ColorButton(vMenuStr[i].c_str(), vMenuColors[i])) {
                    if (nFunIdx >= 0) {
                        vMenuColors[nFunIdx].y = 0.f;
                    }

                    vMenuColors[i].y = 1.f;
                    nFunIdx = i;
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}
