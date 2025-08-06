#pragma once

#include <unordered_set>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class ImguiPage {
public:
    ImguiPage();
    ~ImguiPage();

    void init();
    void mainPage();

    void getPressedKeys();
    void movingCamera();

private:
    int m_nShowType = 0;

    std::unordered_set<std::string> mSetPressedKeys;
};
