#include <stdio.h>
#include <vector>
#include <cstring>
#include <string>
#include <memory>

#include "shader.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "nativefiledialog/nfd.h"

#include <GLFW/glfw3.h>

const int nMaxFileNameLength = 128;
std::shared_ptr< Shader >    g_pt_shader;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow* init_openGL_and_ImGUI(bool bFullScreen = false) {
    // glfw
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return nullptr;
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "title", NULL, NULL);
    if (window == NULL)
        return nullptr;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    // imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if ( bFullScreen ) {
        glfwMaximizeWindow( window );
    }
    
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    // combine
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    return window;
}

void initShader() {
    g_pt_shader = std::make_shared< Shader >(g_sGlslPath + "/glsl/points_shader_rgb.vs", g_sGlslPath + "/glsl/points_shader_rgb.fs");
}

// 主进程
int main(int argc, char** argv) {
    char *cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);
    
    int display_face = 0;
    float Percentage = 1.f;
    bool checkBit = 0;

    GLFWwindow *window = init_openGL_and_ImGUI();
    if (window == nullptr) {
        std::cout << "window is nullptr" << std::endl;
        return -1;
    }
    if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) ) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    initShader();
    // render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // gui
        {
            ImGui::Begin("window title here");

            if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) { }
            ImGui::SameLine();
            if (ImGui::Button("Open File")) {
                nfdresult_t result = NFD_OpenDialog("obj,ply,xyz", nullptr, &cFileName);
                if (result == NFD_OKAY) { }
            }

            if (ImGui::TreeNode("Help")) {
                ImGui::Text( "[H]    | Display/Close main windows" );
                ImGui::TreePop();
                ImGui::Separator();
            }

            if (ImGui::BeginTabBar("tab begin", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("tab title 1"))
                {
                    if (ImGui::Button("show or hide points")) {
                        Eigen::Matrix< double, 4, 4 > proj_mat = Eigen::Matrix< double, 4, 4 >::Identity();
                        Eigen::Matrix< double, 4, 4 > pose_mat = Eigen::Matrix< double, 4, 4 >::Identity();
                        g_pt_shader->loadPoints("/home/channy/Documents/datasets_recon/thermocolorlab/scan000.3d");
                        g_pt_shader->draw(proj_mat, pose_mat);
                    }
                    ImGui::SeparatorText("draw mesh facet or wireframe");

                    ImGui::RadioButton("facet", &display_face, 0);
                    ImGui::RadioButton("wireframe", &display_face, 1);

                    ImGui::SeparatorText("simplify");
                    ImGui::SliderFloat("remain percentage", &Percentage, 1.0, 99.99);
                    ImGui::SeparatorText("normal");
                        
                    if (ImGui::Checkbox(("show xxx"), &checkBit)) {
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("tab title 2")) {
                    if (ImGui::Button("not implement yeah")) {
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}