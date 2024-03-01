#pragma once

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdio.h>

class Common
{
public:
	Common() {}

	static const char* readShader(const std::string& sFileName) {
        char* vShaderStr = new char[120];
        std::ifstream ifs(sFileName.c_str());
        std::stringstream vShaderStream;
        vShaderStream << ifs.rdbuf();
        ifs.close();
        memcpy(vShaderStr, vShaderStream.str().c_str(), 120);
        return vShaderStr;
    }

    
};

