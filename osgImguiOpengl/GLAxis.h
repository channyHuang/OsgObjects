#pragma once

#include "glad/glad.h"

#include <string>
#include <memory>

#include "shader.h"
#include "CameraManager.h"

class GLAxis {
public:
    static GLAxis* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new GLAxis();
		}
		return m_pInstance;
	}

    GLAxis() {}
    virtual ~GLAxis() {
        if (m_bSetBuffer) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(4, m_VBO);
        }
    }

    void init() {
        std::string vertex_shader = std::string( g_sGlslPath + "/glsl/points_shader_rgb.vs" );
        std::string fragment_shader = std::string( g_sGlslPath + "/glsl/points_shader_rgb.fs" );
        m_cShader = std::make_shared<Shader>(vertex_shader, fragment_shader);

        initPts();
        initBuffer();
    }

    void initBuffer() {
        if (m_bSetBuffer) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(4, m_VBO);
        } 
        m_bSetBuffer = true;
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(4, m_VBO);

        glBindVertexArray(m_VAO);
        int nStride = 6 * sizeof( float ) + sizeof(float);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
        glBufferData( GL_ARRAY_BUFFER, m_vPts.size() * ( nStride ), m_vPts.data(), GL_STATIC_DRAW );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, nStride, ( void* ) 0 );
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, nStride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, nStride, (void*)(4 * sizeof(float)));
    }

    void initPts( float fAxisScale = 1 )
    {
        m_vPts.push_back(Point(0, 0, 0, 1.0f, 0.0f, 0.0f));
        m_vPts.push_back(Point(fAxisScale, 0, 0, 1.0f, 0.0f, 0.0f));

        m_vPts.push_back(Point(0, 0, 0, 0.0f, 1.0f, 0.0f));
        m_vPts.push_back(Point(0, fAxisScale, 0, 0.0f, 1.0f, 0.0f));

        m_vPts.push_back(Point(0, 0, 0, 0.0f, 0.0f, 1.0f));
        m_vPts.push_back(Point(0, 0, fAxisScale, 0.0f, 0.0f, 1.0f));
    }

    void draw() {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // Eigen::Matrix< double, 4, 4 > proj_mat = Eigen::Matrix< double, 4, 4 >::Identity();
        // Eigen::Matrix< double, 4, 4 > pose_mat = Eigen::Matrix< double, 4, 4 >::Identity();
        // glm::mat4 projection_mul_view = eigen2glm(proj_mat) * eigen2glm(pose_mat);
        glm::mat4 projection_mul_view = CameraManager::getInstance()->m_glmProjectionMat * eigen2glm(CameraManager::getInstance()->m_camera_pose_mat44_inverse);
        // for (int i = 0; i < 4; ++i) {
        //     for (int j = 0; j < 4; ++j) {
        //         std::cout << projection_mul_view[i][j] << " ";
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << "===================" << std::endl;
        // for (int i = 0; i < 4; ++i) {
        //     for (int j = 0; j < 4; ++j) {
        //         std::cout << CameraManager::getInstance()->m_camera_pose_mat44_inverse(i, j) << " ";
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << "*******************" << std::endl;

        m_cShader->use();
        m_cShader->setUniformFloat( "pointAlpha", 1.0 );
        m_cShader->setUniformFloat( "pointSize", 10 );
        m_cShader->setUniformMat4( "projection_mul_view", projection_mul_view);

        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glPointSize(10);
        glEnable(GL_LINE_WIDTH);
        glLineWidth(10);
        
        glBindVertexArray(m_VAO);

        glDrawArrays(GL_LINES, 0, m_vPts.size());
    }

public:
    static GLAxis* m_pInstance;
    unsigned int m_VAO, m_VBO[4];
    bool m_bSetBuffer = false;
    std::shared_ptr<Shader> m_cShader;
    float m_fAxisScale = 10.f;
    
private:
    std::vector<Point> m_vPts;
};
