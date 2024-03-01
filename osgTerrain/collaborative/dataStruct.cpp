#include "dataStruct.h"

RandomGenerator* RandomGenerator::instance = nullptr;

RandomGenerator* RandomGenerator::getInstance() {
    if (instance == nullptr) {
        instance = new RandomGenerator();
    }
    return instance;
}

RandomGenerator::RandomGenerator() {
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

RandomGenerator::~RandomGenerator() {
}

int RandomGenerator::generateInt(int dim) {
    std::uniform_real_distribution<> dist(0, dim);
    return dist(generator);
}

float RandomGenerator::generateFloat(float maxn) {
    std::uniform_real_distribution<> dist(-maxn, maxn);
    return dist(generator);
}

Eigen::VectorXf  RandomGenerator::generateState(int dim, float maxn) {
    Eigen::VectorXf s(dim);

    std::uniform_real_distribution<> dist(0, maxn);
    for (uint32_t i = 0; i < dim; ++i) {
        s[i] = dist(generator);
    }

    return s;
}

Eigen::VectorXf RandomGenerator::generateAction(int dim, const Eigen::VectorXf& maxn) {
    Eigen::VectorXf a(dim);

    for (uint32_t i = 0; i < dim; ++i) {
        std::uniform_real_distribution<> dist(-maxn[i], maxn[i]);
        a[i] = dist(generator);
    }

    return a;
}

Eigen::MatrixXf RandomGenerator::generateTerrain(int w, int h, int maxn) {
    Eigen::MatrixXf terrain(w, h);
    std::uniform_real_distribution<> dist(0, maxn);
    for (uint32_t i = 0; i < w; ++i) {
        for (uint32_t j = 0; j < h; ++j) {
            terrain(i, j) = dist(generator);
        }
    }
    return terrain;
}

Eigen::VectorXf RandomGenerator::generateVector(int dim, float maxn, bool bothside) {
    Eigen::VectorXf s(dim);
    std::uniform_real_distribution<> dist(bothside ? -maxn : 0, maxn);
    for (uint32_t i = 0; i < dim; ++i) {
        s[i] = dist(generator);
    }
    return s;
}
