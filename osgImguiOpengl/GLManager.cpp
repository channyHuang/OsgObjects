#include "GLManager.h"

#include <stdio.h>
#include "CameraManager.h"
#include "GLMesh.h"

GLManager* GLManager::m_pInstance = nullptr;

GLManager::GLManager() {

}

GLManager::~GLManager() {

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
    switch(eMode) {
        case EDrawMode::MESH:
            GLMesh::getInstance()->init();
            GLMesh::getInstance()->draw();
        break;
        default:
        break;
    }
    return;
    printf("GLManager::draw eMode = %d\n", eMode);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    Eigen::Matrix< double, 4, 4 > proj_mat = Eigen::Matrix< double, 4, 4 >::Identity();
    Eigen::Matrix< double, 4, 4 > pose_mat = Eigen::Matrix< double, 4, 4 >::Identity();
    // glm::mat4 projection_mul_view = eigen2glm(proj_mat) * eigen2glm(pose_mat);
    glm::mat4 projection_mul_view = CameraManager::getInstance()->m_glmProjectionMat * eigen2glm(CameraManager::getInstance()->m_camera_pose_mat44_inverse);

    m_cShader->use();
    m_cShader->setFloat( "pointAlpha", 1.0 );
    m_cShader->setFloat( "pointSize", 10 );
    m_cShader->setMat4( "projection_mul_view", projection_mul_view);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glPointSize(10);
    glEnable(GL_LINE_WIDTH);
    glLineWidth(10);
    
    glBindVertexArray(m_VAO);

    switch (eMode) {
    case EDrawMode::MESH:
        {
            GLMesh::getInstance()->init();
            GLMesh::getInstance()->draw();
        }
        // glDrawArrays(GL_TRIANGLES, 0, m_vPts.data());
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

// pos, normal, color
void GLManager::initBuffer() {
    if (m_bSetBuffer) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    } 
    m_bSetBuffer = true;
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    int nStride = 6 * sizeof( float ) + sizeof( float );

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData( GL_ARRAY_BUFFER, m_vPts.size() * ( nStride ), m_vPts.data(), GL_STATIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, nStride, ( void* ) 0 );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nStride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, nStride, (void*)(3 * sizeof(float)));
}

void GLManager::initShader() {
    std::string vertex_shader = std::string( g_sGlslPath + "/glsl/triangle_facets.vs" );
    std::string fragment_shader = std::string( g_sGlslPath + "/glsl/triangle_facets.fs" );
    m_cShader = std::make_shared<Shader>(vertex_shader.c_str(), fragment_shader.c_str());

    initBuffer();
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
    printf("GLManager::setMesh\n");
    m_vPts.clear();
    m_vPts.push_back(Point(fScale, fScale, fScale, 1.0f, 0.0f, 0.0f));
    m_vPts.push_back(Point(fScale, fScale, -fScale, 1.0f, 0.0f, 0.0f));
    m_vPts.push_back(Point(fScale, -fScale, -fScale, 1.0f, 0.0f, 0.0f));
    m_vPts.push_back(Point(fScale, -fScale, fScale, 1.0f, 0.0f, 0.0f));

    m_vPts.push_back(Point(-fScale, -fScale, fScale, 0.0f, 1.0f, 0.0f));
    m_vPts.push_back(Point(-fScale, fScale, fScale, 0.0f, 1.0f, 0.0f));
    m_vPts.push_back(Point(-fScale, fScale, -fScale, 0.0f, 1.0f, 0.0f));
    m_vPts.push_back(Point(-fScale, -fScale, -fScale, 0.0f, 1.0f, 0.0f));

    m_vMeshIndex.clear();
    m_vMeshIndex = {0, 1, 2, 0, 2, 3, 
                    0, 3, 4, 0, 4, 5,
                    0, 5, 6, 0, 6, 1,
                    7, 5, 4, 7, 6, 5};

    auto calcNormal = [](Point &a, Point& b, Point& c, Point& res) -> void {
        Point v1, v2;
        for (int i = 0; i < 3; ++i) {
            v1.m_pos[i] = a.m_pos[i] - b.m_pos[i];
            v2.m_pos[i] = a.m_pos[i] - c.m_pos[i];
        }
        res.m_pos[0] = v1.m_pos[1] * v2.m_pos[2] - v1.m_pos[2] * v2.m_pos[1];
        res.m_pos[1] = v1.m_pos[2] * v2.m_pos[0] - v1.m_pos[0] * v2.m_pos[2];
        res.m_pos[2] = v1.m_pos[0] * v2.m_pos[1] - v1.m_pos[1] * v2.m_pos[0];
    };

    auto normalize = [](float normal[3]) {
        float norm = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
        if (norm <= 1e-4) return false;
        for (int i = 0; i < 3; ++i) {
            normal[i] /= norm;
        }
        return true;
    };

    for (int i = 0; i < m_vMeshIndex.size(); i += 3) {
        Point res;
        calcNormal(m_vPts[m_vMeshIndex[i]], m_vPts[m_vMeshIndex[i + 1]], m_vPts[m_vMeshIndex[i + 2]], res);
        for (int j = 0; j < 3; ++j) {
            m_vPts[m_vMeshIndex[i]].m_normal[j] += res.m_pos[j];
            m_vPts[m_vMeshIndex[i + 1]].m_normal[j] += res.m_pos[j];
            m_vPts[m_vMeshIndex[i + 2]].m_normal[j] += res.m_pos[j];
        }
        for (int j = 0; j < 3; ++j) {
            normalize(m_vPts[m_vMeshIndex[i + j]].m_normal);
        }
    }

    initBuffer();
}
