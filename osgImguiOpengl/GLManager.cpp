#include "GLManager.h"

#include <stdio.h>
#include "CameraManager.h"

GLManager* GLManager::m_pInstance = nullptr;

GLManager::GLManager() {

}

GLManager::~GLManager() {
    if (m_bSetBuffer) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(4, m_VBO);
    }
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow* GLManager::init(const char* window_title, bool bFullScreen) {
    glfwSetErrorCallback(glfw_error_callback);
    if ( !glfwInit() ) {
        printf("glfwInit failed\n");
        return nullptr;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

    m_pWindow = glfwCreateWindow( 1280, 720, window_title, nullptr, nullptr );
    if ( m_pWindow == nullptr )
        return nullptr;
    CameraManager::getInstance()->bind_gl_callback_function(m_pWindow);
    glfwMakeContextCurrent(m_pWindow);
    glfwSwapInterval(1);

    if ( bFullScreen ) {
        glfwMaximizeWindow( m_pWindow );
    }

    return m_pWindow;
}

void GLManager::draw_frame_start() {
    glClearColor(0, 0, 0, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);
}

void GLManager::draw(EDrawMode eMode) {
    printf("GLManager::draw eMode = %d\n", eMode);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    Eigen::Matrix< double, 4, 4 > proj_mat = Eigen::Matrix< double, 4, 4 >::Identity();
    Eigen::Matrix< double, 4, 4 > pose_mat = Eigen::Matrix< double, 4, 4 >::Identity();
    // glm::mat4 projection_mul_view = eigen2glm(proj_mat) * eigen2glm(pose_mat);
    glm::mat4 projection_mul_view = CameraManager::getInstance()->m_glmProjectionMat * eigen2glm(CameraManager::getInstance()->m_camera_pose_mat44_inverse);

    m_cShader->use();
    m_cShader->setUniformFloat( "pointAlpha", 1.0 );
    m_cShader->setUniformFloat( "pointSize", 10 );
    m_cShader->setUniformMat4( "projection_mul_view", projection_mul_view);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glPointSize(10);
    glEnable(GL_LINE_WIDTH);
    glLineWidth(10);
    
    glBindVertexArray(m_VAO);

    switch (eMode) {
    case EDrawMode::MESH:
        glDrawElements(GL_TRIANGLES, m_vPts.size(), GL_INT, m_vMeshIndex.data());
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        break;
    case EDrawMode::FRAMEWARE:
        glDrawArrays(GL_TRIANGLES, 0, 0);
        break;
    case EDrawMode::LINE:
        glDrawArrays(GL_LINES, 0, m_vPts.size());
        break;
    case EDrawMode::POINT:
    default:
        glDrawArrays(GL_POINTS, 0, m_vPts.size());
        break;
    }
    glDepthFunc( GL_LESS );
}

void GLManager::initBuffer() {
    if (m_bSetBuffer) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(4, m_VBO);
    } 
    m_bSetBuffer = true;
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(4, m_VBO);

    glBindVertexArray(m_VAO);
    int nStride = 6 * sizeof( float ) + sizeof( float );

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glBufferData( GL_ARRAY_BUFFER, m_vPts.size() * ( nStride ), m_vPts.data(), GL_STATIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, nStride, ( void* ) 0 );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, nStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, nStride, (void*)(4 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[1]);
    glBufferData( GL_ARRAY_BUFFER, m_vMeshIndex.size() * ( sizeof(int) ), m_vPts.data(), GL_STATIC_DRAW );
    // glEnableVertexAttribArray( 1 );
    // glVertexAttribPointer( 1, 1, GL_FLOAT, GL_FALSE, nStride, ( void* ) 0 );

    // glBindBuffer(GL_ARRAY_BUFFER, m_VBO[2]);
    // glBufferData( GL_ARRAY_BUFFER, m_vPts.size() * ( nStride ), m_vPts.data() + 4 * sizeof(float), GL_STATIC_DRAW );
    // glEnableVertexAttribArray( 2 );
    // glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, nStride, ( void* ) 0 );

    // glBindBuffer(GL_ARRAY_BUFFER, m_VBO[3]);
    // glBufferData( GL_ARRAY_BUFFER, m_vPts.size() * ( nStride ), m_vPts.data() + 7 * sizeof(float), GL_STATIC_DRAW );
    // glEnableVertexAttribArray( 3 );
    // glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, nStride, ( void* ) 0 );
}

void GLManager::initShader() {
    std::string vertex_shader = std::string( g_sGlslPath + "/glsl/points_shader_rgb.vs" );
    std::string fragment_shader = std::string( g_sGlslPath + "/glsl/points_shader_rgb.fs" );
    m_cShader = std::make_shared<Shader>(vertex_shader, fragment_shader);
}

void GLManager::setPts(float fScale) {
    m_vPts.clear();
    m_vPts.push_back(Point(0, 0, 0, 1.0f, 0.0f, 0.0f));
    m_vPts.push_back(Point(fScale, 0, 0, 1.0f, 0.0f, 0.0f));

    m_vPts.push_back(Point(0, 0, 0, 0.0f, 1.0f, 0.0f));
    m_vPts.push_back(Point(0, fScale, 0, 0.0f, 1.0f, 0.0f));

    m_vPts.push_back(Point(0, 0, 0, 0.0f, 0.0f, 1.0f));
    m_vPts.push_back(Point(0, 0, fScale, 0.0f, 0.0f, 1.0f));

    initBuffer();
}

void GLManager::setMesh(float fScale) {
    m_vPts.clear();
    m_vPts.push_back(Point(fScale, fScale, fScale, 1.0f, 0.0f, 0.0f));
    m_vPts.push_back(Point(fScale, fScale, -fScale, 1.0f, 0.0f, 0.0f));
    m_vPts.push_back(Point(fScale, -fScale, -fScale, 1.0f, 0.0f, 0.0f));
    m_vPts.push_back(Point(fScale, -fScale, fScale, 1.0f, 0.0f, 0.0f));

    m_vPts.push_back(Point(-1, -1, 1, 0.0f, 1.0f, 0.0f));
    m_vPts.push_back(Point(-1, 1, 1, 0.0f, 1.0f, 0.0f));
    m_vPts.push_back(Point(-1, 1, -1, 0.0f, 1.0f, 0.0f));
    m_vPts.push_back(Point(-1, -1, -1, 0.0f, 1.0f, 0.0f));

    m_vMeshIndex.clear();
    m_vMeshIndex = {0, 1, 2, 0, 2, 3, 
                    0, 3, 4, 0, 4, 5,
                    0, 5, 6, 0, 6, 1,
                    7, 5, 4, 7, 6, 5};

    initBuffer();
}
