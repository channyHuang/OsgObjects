#include <iostream>

#include "ImguiPage.h"
#include "GLManager.h"
#include "CameraManager.h"

#include "GLAxis.h"

int main() {
	GLFWwindow *pWindow = GLManager::getInstance()->init("title");
	if (pWindow == nullptr) {
        std::cout << "window is nullptr" << std::endl;
        return -1;
    }

    ImguiPage cImguiPage;
    cImguiPage.init();

    ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
    ImGui_ImplOpenGL3_Init(GLManager::getInstance()->glsl_version);
	
    CameraManager::getInstance()->set_windows_projection_matrix(2000.f, CameraManager::getInstance()->m_fZNear, CameraManager::getInstance()->m_fZFar);

    const GLFWvidmode* vidmode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
    CameraManager::getInstance()->set_windows_projection_matrix(vidmode->width / ( 3840.0 / 2000.0 ), CameraManager::getInstance()->m_fZNear, CameraManager::getInstance()->m_fZFar);
    
    if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) ) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // shader
    GLManager::getInstance()->initShader();
    GLAxis::getInstance()->init();

    // frame update
	while (!glfwWindowShouldClose(pWindow)) {
        // frame start
        glfwPollEvents();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		cImguiPage.mainPage();

        // draw axis
        GLAxis::getInstance()->draw();

        CameraManager::getInstance()->update();
        // frame finish
		glfwSwapBuffers(pWindow);
	}

	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(pWindow);
    glfwTerminate();

	return 0;
}
