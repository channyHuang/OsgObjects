#include "CameraManager.h"

#include "tools/lib_sophus/se3.hpp"

CameraManager* CameraManager::m_pInstance = nullptr;

CameraManager::CameraManager() {
    refresh_pose();
}

CameraManager::~CameraManager() {}

void CameraManager::refresh_pose() {
    m_camera_pose_mat44 = Sophus::SE3f( m_camera_rot, m_camera_pos ).matrix();
    m_camera_pose_mat44_inverse = m_camera_pose_mat44.inverse();
}

void CameraManager::framebuffer_size_callback(GLFWwindow* pWindow, int nWidth, int nHeight) {
    set_windows_projection_matrix(m_fFocus, m_fZNear, m_fZFar);
}

// 摄像机移动
void CameraManager::scroll_callback(GLFWwindow* pWindow,  float fXOffset, float fYOffset) {
    Eigen::Vector3f translation = m_fLastCursorDepth * Eigen::Vector3f(0, 0, -fYOffset * 0.1);
    translation = m_camera_rot * translation;
    m_camera_pos += translation;
}

float getTruthDepth(float fZRead, float fZNear, float fZFar) {
    float fZN = 2.0 * fZRead - 1.0;
    return 2.0 * fZNear * fZFar / (fZFar + fZNear - fZN * (fZFar - fZNear));
}

// 获取屏幕点深度
float CameraManager::getScreenPixelDepth(int nPosx, int nPosy) {
    float fZRead = 0;
    nPosy = m_nHeight - nPosy;
    if (nPosx < 1 || (nPosx > m_camera_intrinsic(0, 2) * 2 - 1)) return m_fZFar;
    if (nPosy < 1 || (nPosy > m_camera_intrinsic(1, 2) * 2 - 1)) return m_fZFar;
    glReadPixels(nPosx, nPosy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fZRead);
    return getTruthDepth(fZRead, m_fZNear, m_fZFar);
}

// 反投影屏幕点到三维点
Eigen::Vector3f CameraManager::unprojectPoint(float fx, float fy, float fDepth) {
    Eigen::Vector3f vPtScreen = m_camera_intrinsic_inv * Eigen::Vector3f(fx, fy, 1);
    vPtScreen = vPtScreen * fDepth / vPtScreen(2);
    Eigen::Vector3f vPtCamera = Eigen::Vector3f(vPtScreen(0), -vPtScreen(1), -vPtScreen(2));
    return m_camera_rot * vPtCamera + m_camera_pos;
}

// 获取屏幕点的三维坐标
Eigen::Vector3f CameraManager::getCursorClickedPts() {
    Eigen::Vector3f pts(3e8, 3e8, 3e8);
    float fDepth = getScreenPixelDepth(m_ImguiIo->MousePos.x, m_ImguiIo->MousePos.y);
    if (fDepth > 0.8f * m_fZFar) {
        fDepth = m_fLastCursorDepth;
    }
    m_fLastCursorDepth = fDepth;
    m_fDragPtDepth = fDepth;
    return unprojectPoint(m_ImguiIo->MousePos.x, m_ImguiIo->MousePos.y, fDepth);
}

// 鼠标点击事件
void CameraManager::mouse_button_callback(GLFWwindow* pWindow, int nButton, int nAction, int nMods) {
    if (m_ImguiIo->WantCaptureMouse) {
        return;
    }

    m_vDragStartScreen = Eigen::Vector2f(m_ImguiIo->MousePos.x, m_ImguiIo->MousePos.y);
    m_vDragCameraPos = m_camera_pos;
    m_matDragCameraRot = m_camera_rot;
    m_vDragPtInWorld = getCursorClickedPts();
    if (m_vDragPtInWorld(0) == 3e8) {
        m_vDragPtInWorld = unprojectPoint(m_ImguiIo->MousePos.x, m_ImguiIo->MousePos.y, m_fDragPtDepth);
    }
}

void CameraManager::mouse_callback(GLFWwindow* pWindow, float fx, float fy) {
    m_bLeftButton = m_ImguiIo->MouseDown[0];
    m_bRightButton = m_ImguiIo->MouseDown[1];
    m_bMiddleButton = m_ImguiIo->MouseDown[2];

    float fMaxRotSpeed = 1;
    if (m_bLeftButton && m_bRightButton) {
        float fRoll = (m_ImguiIo->MousePos.x - m_vDragStartScreen(0)) * m_fRotateSensitive;
        m_camera_rot = m_matDragCameraRot * Sophus::SO3f::exp(Eigen::Vector3f(0, 0, fRoll) / 57.31).matrix();

        Eigen::Vector3f vPtScreen = m_camera_intrinsic_inv * Eigen::Vector3f(m_vDragStartScreen(0), m_vDragStartScreen(1), 1);
        Eigen::Vector3f vPtGlCam = Eigen::Vector3f(vPtScreen(0), -vPtScreen(1), -vPtScreen(2));
        m_camera_pos = m_vDragPtInWorld - m_camera_rot * vPtGlCam;
    } else if (m_bRightButton || m_bMiddleButton) {

    } else if (m_bLeftButton) {
        float fRotateYaw = (m_ImguiIo->MousePos.x - m_vDragStartScreen(0)) * m_fRotateSensitive;
        float fRotatePitch = (m_ImguiIo->MousePos.y - m_vDragStartScreen(1)) * m_fRotateSensitive;
        Eigen::Matrix<float, 3, 3> matTmp = Sophus::SO3f::rotZ(-fRotateYaw / DEGREEINPI).matrix() * m_matDragCameraRot * Sophus::SO3f::rotX(-fRotatePitch / DEGREEINPI).matrix();
        m_camera_rot = Eigen::Quaternionf(matTmp).normalized().toRotationMatrix();
        Eigen::Vector3f vPtInScreen = m_camera_intrinsic_inv * Eigen::Vector3f(m_vDragStartScreen(0), m_vDragStartScreen(1), 1);
        vPtInScreen = vPtInScreen * m_fDragPtDepth / vPtInScreen(2);
        m_camera_pos = m_vDragPtInWorld - m_camera_rot * Eigen::Vector3f(vPtInScreen(0), -vPtInScreen(1), -vPtInScreen(2));
    }
}

void CameraManager::bind_gl_callback_function(GLFWwindow* pWindow) {
    m_pWindow = pWindow;

    glfwSetWindowUserPointer(pWindow, this);
    auto framebuffer_size_callback_func = []( GLFWwindow* w, int width, int height ) {
        static_cast< CameraManager* >( glfwGetWindowUserPointer( w ) )->framebuffer_size_callback( w, width, height );
    };
    auto mouse_button_callback_func = []( GLFWwindow* w, int button, int action, int mods ) {
        static_cast< CameraManager* >( glfwGetWindowUserPointer( w ) )->mouse_button_callback( w, button, action, mods );
    };

    auto mouse_callback_func = []( GLFWwindow* w, double xpos_In, double ypos_In ) {
        static_cast< CameraManager* >( glfwGetWindowUserPointer( w ) )->mouse_callback( w, xpos_In, ypos_In );
    };
    auto scroll_callback_func = []( GLFWwindow* w, double x_offset, double yoffset ) {
        static_cast< CameraManager* >( glfwGetWindowUserPointer( w ) )->scroll_callback( w, x_offset, yoffset );
    };
    glfwSetFramebufferSizeCallback( m_pWindow, framebuffer_size_callback_func );
    glfwSetMouseButtonCallback( m_pWindow, mouse_button_callback_func );
    glfwSetCursorPosCallback( m_pWindow, mouse_callback_func );
    glfwSetScrollCallback( m_pWindow, scroll_callback_func );
}

void CameraManager::set_gl_projection(int nWidth, int nHeight, float fFocus) {
    if (nWidth != 0 && nHeight != 0) {
        m_nWidth = nWidth;
        m_nHeight = nHeight;
    } 
    if (fFocus != 0) {
        m_fFocus = fFocus;
    }

    float fFovY = 2 * atan2(m_nHeight / 2, m_fFocus);
    m_glmProjectionMat = glm::perspective(fFovY, (float)m_nWidth / m_nHeight, m_fZNear, m_fZFar);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&m_glmProjectionMat[0][0]);
    glViewport(0, 0, m_nWidth, m_nHeight);
}

void CameraManager::update() {
    glMatrixMode(GL_MODELVIEW);
    refresh_pose();
    glLoadMatrixf(m_camera_pose_mat44_inverse.data());
}

void CameraManager::set_windows_projection_matrix(float fFocus, float fZNear, float fZFar) {
    glfwGetFramebufferSize(m_pWindow, &m_nWidth, &m_nHeight);
    m_camera_intrinsic << fFocus, 0, m_nWidth / 2, 0, fFocus, m_nHeight / 2, 0, 0, 1;
    m_camera_intrinsic_inv = m_camera_intrinsic.inverse();
    m_fFocus = fFocus;

    set_gl_projection(0, 0, m_fFocus);
}

void CameraManager::move(Direction eDir) {
    float fDistance = m_fMovingSensitive * m_fClickedDepth;
    switch(eDir) {
    case Direction::Up:
        m_camera_pos += (m_camera_rot.col(1) * fDistance);
        break;
    case Direction::Down:
        m_camera_pos -= (m_camera_rot.col(1) * fDistance);
        break;
    default:
        break;
    }
}