#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

#include "shader.h"

enum class EDrawMode : int {
	POINT = 0,
	LINE = 1,
	MESH = 2,
	FRAMEWARE = 3
};

class GLManager {
public:
    static GLManager* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new GLManager();
		}
		return m_pInstance;
	}

	~GLManager();

	GLFWwindow* init(const char* window_title, bool bFullScreen = false);

	void initBuffer();
	void initShader();
	void setPts(float fScale = 10);
	void setMesh(float fScale = 10);

	void draw(EDrawMode eMode = EDrawMode::POINT);

	void draw_frame_start();

public:
	const char* glsl_version = "#version 330";

private:
    GLManager();

private:
    static GLManager* m_pInstance;
	GLFWwindow* m_pWindow = nullptr;

	unsigned int m_VAO, m_VBO[4];
	bool m_bSetBuffer = false;

	std::vector<Point> m_vPts;
	std::vector<int> m_vMeshIndex;
    std::shared_ptr<Shader> m_cShader;
};
