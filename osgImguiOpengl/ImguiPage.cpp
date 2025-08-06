#include "ImguiPage.h"

#include "osgManager.h"
#include "CameraManager.h"

ImguiPage::ImguiPage() {}
ImguiPage::~ImguiPage() {}

void ImguiPage::init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    CameraManager::getInstance()->initImguiIo(&io);
    ImGui::StyleColorsDark();
}

void ImguiPage::mainPage() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("window title here");

    if (ImGui::BeginTabBar("tab begin", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("tab title 1")) {
            ImGui::RadioButton("Points", &m_nShowType, 0);
            ImGui::RadioButton("Lines", &m_nShowType, 1);
            ImGui::RadioButton("Mesh", &m_nShowType, 2);
            ImGui::RadioButton("Frameware", &m_nShowType, 3);
            if (ImGui::Button("updateShow")) {
                OsgManager::getInstance()->updateShow(m_nShowType);
            }
        
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

struct imgui_key_funcs {
    static bool IsLegacyNativeDupe( ImGuiKey key ) {
        return key < 512 && ImGui::GetIO().KeyMap[ key ] != -1;
    }
};

void ImguiPage::getPressedKeys() {
    mSetPressedKeys.clear();
    for (int key = 0; key < (int)ImGuiKey_COUNT; key++) {
        if (imgui_key_funcs::IsLegacyNativeDupe( (ImGuiKey )key )) continue;
        if ( ImGui::IsKeyPressed( (ImGuiKey )key ) ) {
            mSetPressedKeys.insert(ImGui::GetKeyName((ImGuiKey)key));
        }
    }
}

void ImguiPage::movingCamera() {
    if (mSetPressedKeys.find("UpArrow") != mSetPressedKeys.end()) {
        CameraManager::getInstance()->move(Direction::Up);
    } else if (mSetPressedKeys.find("DownArrow") != mSetPressedKeys.end()) {
        CameraManager::getInstance()->move(Direction::Down);
    }
}
