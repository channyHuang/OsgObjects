#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui/imgui.h"
#include <Eigen/Core>

enum Direction {
    Up = 0,
    Down,
    Left,
    Right
};

#define DEGREEINPI (180 / 3.1415926)

class CameraManager {
public:
	static CameraManager* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new CameraManager();
		}
		return m_pInstance;
	}

	virtual ~CameraManager();

    void initImguiIo(ImGuiIO* io) {
        m_ImguiIo = io;
    }

    void bind_gl_callback_function(GLFWwindow* pWindow);
    void framebuffer_size_callback(GLFWwindow* pWindow, int nWidth, int nHeight);
    void scroll_callback(GLFWwindow* pWindow,  float fXOffset, float fYOffset); 
    void mouse_button_callback(GLFWwindow* pWindow, int nButton, int nAction, int nMods);
    void mouse_callback(GLFWwindow* pWindow, float fx, float fy);
    
    void set_gl_projection(int nWidth = 0, int nHeight = 0, float fFocus = 0);
    void set_windows_projection_matrix(float fFocus, float fZNear, float fZFar);

    float m_fZNear = 0.1f;
    float m_fZFar = 1500;
    glm::mat4 m_glmProjectionMat; 

    Eigen::Vector3f               m_camera_pos = Eigen::Vector3f( 0, 0, 1 );
    Eigen::Matrix< float, 3, 3 >  m_camera_rot = Eigen::Matrix< float, 3, 3 >::Identity();
    Eigen::Matrix< float, 4, 4 >  m_camera_pose_mat44;
    Eigen::Matrix< float, 4, 4 >  m_camera_pose_mat44_inverse;
    Eigen::Matrix< float, 3, 3 >  m_camera_intrinsic;
    Eigen::Matrix< float, 3, 3 >  m_camera_intrinsic_inv;

    void move(Direction eDir = Direction::Up);
    void update();

protected:
    CameraManager();

private:
    void refresh_pose();
    float getScreenPixelDepth(int nPosx, int nPosy);
    Eigen::Vector3f unprojectPoint(float fx, float fy, float fDepth);
    Eigen::Vector3f getCursorClickedPts();

protected:
	static CameraManager* m_pInstance;

private:
    float m_fFocus = 3000.f;
    int m_nWidth = 640, m_nHeight = 480;
    GLFWwindow* m_pWindow;
    ImGuiIO* m_ImguiIo = nullptr;
    bool m_bLeftButton = false, m_bRightButton = false, m_bMiddleButton = false;

    // 旋转平移灵敏度
    float m_fMovingSensitive = 0.1f;
    float m_fRotateSensitive = 0.15f;

    // 输入事件
    float m_fClickedDepth = 1.f;
    float m_fLastCursorDepth = 1.0f;
    // 
    float m_fDragPtDepth = 1.f;
    Eigen::Vector2f m_vDragStartScreen;
    Eigen::Vector3f m_vDragPtInWorld;
    Eigen::Vector3f m_vDragCameraPos;
    Eigen::Matrix<float, 3, 3> m_matDragCameraRot;
};
