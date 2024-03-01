#include <stdio.h>
#include <vector>
#include <cstring>
#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#define GLEW_STATIC
#include <gl/glew.h>
#include <glfw/glfw3.h>

#include "nativefiledialog/nfd.h"

#include "commonFunc.h"

GLuint vao[1];
GLuint program;
GLuint textureID;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void createTexture(const unsigned char* data, int width, int height) {
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
}

GLuint createShaderProgram() {
    const char* vShaderSource = Common::readShader("./glsl/vshader.glsl");
    const char* fShaderSource = Common::readShader("./glsl/fshader.glsl");
    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vShaderSource, nullptr);
    glCompileShader(vshader);

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fShaderSource, nullptr);
    glCompileShader(fshader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    return program;
}

void init(GLFWwindow* window) {
    program = createShaderProgram();
    //glGenVertexArrays(1, vao);
    //glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window) {
    glUseProgram(program);
    glPointSize(30.f);
    glDrawArrays(GL_POINTS, 0, 1);
}

const int nMaxFileNameLength = 128;

int main(int argc, char** argv) {
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.0f);

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "PointCloud Reconstruction", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);

    int state = glewInit();
    if (state != GLEW_OK) {
        std::cout << " init failed" << std::endl;
    }

    glfwSwapInterval(1);
    init(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    
    char *cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);
    int radio = 0;
    float Percentage = 1.f;
    bool checkBit = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("point cloud reconstruction");

            if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
            }
            ImGui::SameLine();
            if (ImGui::Button("Open File")) {
                nfdresult_t result = NFD_OpenDialog("obj,ply,xyz", nullptr, &cFileName);
                if (result == NFD_OKAY) {
                }
            }
            if (ImGui::Button("show model")) {
            }
            
            if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Reconstruction"))
                {
                    if (ImGui::Button("cluster")) {
                    }
                    ImGui::SeparatorText("smooth");
                    for (int i = 0; i < 3; ++i) {
                        ImGui::RadioButton(("radio" + std::to_string(i)).c_str(), &radio, i);
                    }
                    ImGui::SeparatorText("simplify");
                    ImGui::SliderFloat("remain percentage", &Percentage, 1.0, 99.99);
                    ImGui::SeparatorText("normal");
                    ImGui::SeparatorText("normalOri");
                    ImGui::SeparatorText("Reconstruct");
                    for (int i = 0; i < 3; ++i) {
                        if (ImGui::Checkbox(("checkbox" + std::to_string(i)).c_str(), &checkBit)) {
                        }
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Unwrap"))
                {
                    if (ImGui::Button("show charts")) {
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("test"))
                {
                    if (ImGui::Button("show")) {
                    }

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        display(window);

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}