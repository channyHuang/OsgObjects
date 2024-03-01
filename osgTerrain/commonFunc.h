#pragma once

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <random>
#include <iosfwd>
#include <chrono>

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
        std::cout << __FUNCTION__ << " read shader : " << vShaderStr << std::endl;
        return vShaderStr;
    }

    static std::string genUUID() {
        std::stringstream stream;
        auto random_seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 seed_engine(random_seed);
        std::uniform_int_distribution<std::size_t> random_gen;
        std::size_t value = random_gen(seed_engine);
        stream << std::hex << value;
        return stream.str();
    }
};

