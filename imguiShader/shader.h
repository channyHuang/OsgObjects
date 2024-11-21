#pragma once

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include "./glad/glad.h"
#include <Eigen/Core>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef GLSL_PATH
std::string g_sGlslPath = GLSL_PATH;
#endif

struct Point {
    float m_pos[3];
    float m_color = 0x000000FF;
    Point() = default;

    void pack_color(const unsigned char r, const unsigned char g, const unsigned char b)
    {
        m_color =  (r << 16) | (g << 8) | b;
    }

    void pack_color(const float rf, const float gf, const float bf)
    {
        unsigned char r = (unsigned char)(rf * 255);
        unsigned char g = (unsigned char)(gf * 255);
        unsigned char b = (unsigned char)(bf * 255);
        m_color =  (r << 16) | (g << 8) | b;
    }

    Point( const float& x, const float& y, const float& z, const float& rf, const float& gf, const float& bf )
    {
        m_pos[ 0 ] = x;
        m_pos[ 1 ] = y;
        m_pos[ 2 ] = z;
        pack_color( ( float ) rf, ( float ) gf, ( float ) bf );
    }

    Point( const float& x, const float& y, const float& z, const unsigned char& r, const unsigned char& g, const unsigned char& b )
    {
        m_pos[ 0 ] = x;
        m_pos[ 1 ] = y;
        m_pos[ 2 ] = z;
        pack_color( r, g, b );
    }
};

template < typename T >
inline glm::mat4 eigen2glm( const Eigen::Matrix< T, 4, 4 >& eigen_mat )
{
    glm::mat4 temp_mat;
    for ( int i = 0; i < 4; i++ )
    {
        for ( int j = 0; j < 4; j++ )
        {
            temp_mat[ i ][ j ] = eigen_mat( j, i );
        }
    }
    return temp_mat;
}

std::vector<std::string> splitString(std::string& str, char c) {
	std::vector<std::string> res;
	size_t stpos = 0;
	while (stpos != std::string::npos) {
		size_t nextpos = str.find_first_of(c, stpos);
		if (nextpos != std::string::npos) {
			res.push_back(str.substr(stpos, nextpos - stpos));

			stpos = nextpos + 1;
		}
		else {
			res.push_back(str.substr(stpos));
			break;
		}
	}
	return res;
}

class Shader
{
public:
	Shader(const std::string& sVertexShaderFile, const std::string& sFragmentShaderFile) {
        loadShader(sVertexShaderFile, sFragmentShaderFile);
    }
    ~Shader() {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    }

    // 读取单个shader文件
    std::string readShader(const std::string& sShaderFile) {
        std::string sShaderStr;
        std::ifstream ifs(sShaderFile.c_str(), std::ios::in);
        std::stringstream sShaderStream;
        sShaderStream << ifs.rdbuf();
        ifs.close();
        sShaderStr = sShaderStream.str();
        return sShaderStr;
    }

    bool checkCompileErrors(GLuint nShaderLoc, std::string sType) {
        GLint nSuccess = 0;
        GLchar cInfoLog[1024];
        if (sType != "PROGRAM") {
            glGetShaderiv(nShaderLoc, GL_COMPILE_STATUS, &nSuccess);
            if (!nSuccess) {
                glGetShaderInfoLog(nShaderLoc, 1024, nullptr, cInfoLog);
                std::cout << "error: shader compile " << sType << std::endl << " " << cInfoLog << std::endl;
                return false;
            }
        } else {
            glGetProgramiv(nShaderLoc, GL_LINK_STATUS, &nSuccess);
            if (!nSuccess) {
                glGetProgramInfoLog(nShaderLoc, 1024, nullptr, cInfoLog);
                std::cout << "error: shader compile " << sType << std::endl << cInfoLog << std::endl;
                return false;
            }
        }
        return true;
    }

    // 加载vshader和fshader
    void loadShader(const std::string& sVertexShaderFile, const std::string& sFragmentShaderFile) {
        std::ifstream ifsVertex(sVertexShaderFile.c_str(), std::ios::in);
        std::ifstream ifsFragment(sFragmentShaderFile.c_str(), std::ios::in);
        std::stringstream sVertexStream, sFragmentStream;
        sVertexStream << ifsVertex.rdbuf();
        sFragmentStream << ifsFragment.rdbuf();
        ifsVertex.close();
        ifsFragment.close();
        std::string sVertexCode = sVertexStream.str();
        std::string sFragmentCode = sFragmentStream.str();

        const char* pVertexCode = sVertexCode.c_str();
        const char* pFragmentCode = sFragmentCode.c_str();

        GLuint nVertexLoc = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(nVertexLoc, 1, &pVertexCode, nullptr);
        glCompileShader(nVertexLoc);
        bool res = checkCompileErrors(nVertexLoc, "VERTEX");

        GLuint nFragmentLoc = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(nFragmentLoc, 1, &pFragmentCode, nullptr);
        glCompileShader(nFragmentLoc);
        res = checkCompileErrors(nFragmentLoc, "FRAGMENT");

        m_nProgramID = glCreateProgram();
        glAttachShader(m_nProgramID, nVertexLoc);
        glAttachShader(m_nProgramID, nFragmentLoc);
        glLinkProgram(m_nProgramID);
        res = checkCompileErrors(m_nProgramID, "PROGRAM");

        glDeleteShader(nVertexLoc);
        glDeleteShader(nFragmentLoc);
    }

    void use() {
        glUseProgram(m_nProgramID);
    }

    void setUniformFloat(const std::string& sName, float value) {
        glUniform1f(glGetUniformLocation(m_nProgramID, sName.c_str()), value);
    }

    void setUniformMat4(const std::string& sName, const glm::mat4& value) {
        glUniformMatrix4fv(glGetUniformLocation(m_nProgramID, sName.c_str()), 1, GL_FALSE, &value[0][0]);
    }

    void init_data_buffer() {
        if (m_bInitBuffer) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
        }
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
        int stride = 3 * sizeof(float) + sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, m_vPoints.size() * (stride), m_vPoints.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        m_bInitBuffer = true;
    }

    void draw( const Eigen::Matrix< double, 4, 4 > proj_mat, const Eigen::Matrix< double, 4, 4 > pose_mat, GLenum eMode = GL_POINTS) {
        int nCount = m_vPoints.size();
        if (nCount <= 0) return;
        if (m_bInitBuffer == false) {
            init_data_buffer();
            if (m_bInitBuffer == false) {
                std::cout << __LINE__ << " init failed " << std::endl;
                return;
            }
        }

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        glm::mat4 projection_mul_view = eigen2glm(proj_mat) * eigen2glm(pose_mat);
        projection_mul_view[0][0] = 2.18341;
        projection_mul_view[1][1] = 1.9685;
        projection_mul_view[2][2] = -1.00013;
        projection_mul_view[2][3] = -1;
        projection_mul_view[3][2] = 19.8027;
        projection_mul_view[3][3] = 20;

        use();
        setUniformFloat("pointAlpha", 1.0);
        setUniformFloat("pointSize", 10);
        setUniformMat4("projection_mul_view", projection_mul_view);

        if (eMode == GL_POINTS) {
            glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
            glPointSize(10);
        } else {
            glEnable(GL_LINE_WIDTH);
            glLineWidth(10);
        }

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_POINTS, 0, nCount);
        glDepthFunc(GL_LESS);
    }

    void loadPoints(const std::string& sFileName) {
        if (m_vPoints.size() > 0) {
            m_vPoints.clear();
        }
        std::ifstream ifs(sFileName, std::ios::in);
        std::string sLine;
        while (!ifs.eof()) {
            std::getline(ifs, sLine);
            std::vector<std::string> data = splitString(sLine, ' ');
            if (data.size() < 7) continue;
            Point pt = Point(std::atof(data[0].c_str()) * 100, std::atof(data[1].c_str()) * 100, std::atof(data[2].c_str()) * 100, (unsigned char)std::atoi(data[4].c_str()), (unsigned char)std::atoi(data[5].c_str()), (unsigned char)std::atoi(data[6].c_str()));
            m_vPoints.push_back(pt);
        }
        ifs.close();
        std::cout << __LINE__ << " " << m_vPoints.size() << std::endl;
    }

public:
    GLuint m_nProgramID;
    GLuint m_VAO, m_VBO;
    bool m_bInitBuffer = false;
    std::vector<Point> m_vPoints;
};
