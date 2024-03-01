#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>

#include <Eigen/Core>

static const float epson = 1e-4;

static void output(const std::string& name, const Eigen::VectorXf& v) {
    std::cout << name << ":";
    int len = v.size();
    for (int i = 0; i < len; ++i) {
        std::cout << " " << v(i);
        if (i == 1) std::cout << " ->";
        else if (i == 3) std::cout << " ||";
    }
    std::cout << std::endl;
}

static void output2File(const std::string& filename, const Eigen::VectorXf& v) {
    std::ofstream ofs;
    ofs.open(filename.c_str(), std::ios_base::app);
    ofs << v[0] << "\t" << v[1] << "\t" << v[2] << "\t" << v[3] << std::endl;
    ofs.close();
}

static bool isValidPos(const Eigen::VectorXf& v, float maxn) {
    int len = v.size();
    for (int i = 0; i < len; ++i) {
        if (v[i] < 0 || v[i] >= maxn) return false;
    }
    return true;
}

static bool isZero(float v) {
    return (fabs(v) < epson);
}

#endif // COMMON_H
