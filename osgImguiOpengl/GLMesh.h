#pragma once

#include "glad/glad.h"

#include <string>
#include <memory>

#include "shader_m.h"
#include "CameraManager.h"
#include "shader.h"

class GLMesh {
public:
    static GLMesh* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new GLMesh();
		}
		return m_pInstance;
	}

    GLMesh() {}
    virtual ~GLMesh() {
        if (m_bSetBuffer) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
        }
    }

    void init() {
        std::string vertex_shader = std::string( g_sGlslPath + "/glsl/triangle_facets.vs" );
        std::string fragment_shader = std::string( g_sGlslPath + "/glsl/triangle_facets.fs" );
        m_cShader = std::make_shared<Shader>(vertex_shader.c_str(), fragment_shader.c_str());
    }

    void initData( float fAxisScale = 1 ) {
        m_vPts.push_back(Point(fAxisScale, 0, 0, 1.0f, 0.0f, 0.0f));
        m_vPts.push_back(Point(0, fAxisScale, 0, 0.0f, 1.0f, 0.0f));
        m_vPts.push_back(Point(0, 0, fAxisScale, 0.0f, 0.0f, 1.0f));

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                m_vPts[i].m_normal[j] = 0.33;
            }
            m_vIndex.push_back(i);
        }
    }

    void initBuffer() {
        if (m_bSetBuffer) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
        } 
        m_bSetBuffer = true;
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        int nStride = 6 * sizeof( float ) + sizeof(float);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData( GL_ARRAY_BUFFER, m_vPts.size() * ( nStride ), m_vPts.data(), GL_STATIC_DRAW );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, nStride, ( void* ) 0 );
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nStride, (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, nStride, (void*)(3 * sizeof(float)));
    }

    void draw() {
        initData(20);
        initBuffer();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        Eigen::Matrix< double, 4, 4 > matIdentity = Eigen::Matrix< double, 4, 4 >::Identity();
        glm::mat4 projection_mul_view = CameraManager::getInstance()->m_glmProjectionMat * eigen2glm(CameraManager::getInstance()->m_camera_pose_mat44_inverse);

        m_cShader->use();
        m_cShader->setBool( "if_light", false );
        m_cShader->setVec3( "lightPos",  0.0, 0.0, 0.0);
        m_cShader->setVec3( "lightColor",  1.0, 0.0, 0.0);
        m_cShader->setMat4( "projection", projection_mul_view);
        m_cShader->setMat4( "view", matIdentity);

        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glPointSize(10);
        glEnable(GL_LINE_WIDTH);
        glLineWidth(10);

        glBindVertexArray(m_VAO);
        
        glDrawElements(GL_TRIANGLES, m_vIndex.size() / 3, GL_UNSIGNED_INT, m_vIndex.data());
    }

public:
    static GLMesh* m_pInstance;
    unsigned int m_VAO, m_VBO;
    bool m_bSetBuffer = false;
    std::shared_ptr<Shader> m_cShader;
    
private:
    std::vector<Point> m_vPts;
    std::vector<unsigned int> m_vIndex;
};
