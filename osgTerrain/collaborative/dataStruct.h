#ifndef DATASTRUCTION_H
#define DATASTRUCTION_H

#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>

#include "Eigen/Core"

class MyTuples {
public:
    MyTuples() {}
    MyTuples(uint32_t nStateDim, uint32_t nActionDim) {
        state.resize(nStateDim);
        stateNext.resize(nStateDim);
        action.resize(nActionDim);
    }
    MyTuples(uint32_t id, float reward, Eigen::VectorXf _state, Eigen::VectorXf _action, Eigen::VectorXf _stateNext) {
        nId = id;
        fReward = reward;
        state = _state;
        action = _action;
        stateNext = _stateNext;
    }

    uint32_t nId;
    float fReward = 0.f;
    Eigen::VectorXf state;
    Eigen::VectorXf action;
    Eigen::VectorXf stateNext;
    float qValue = 0.f;
    float qNextValue = 0.f;
};

class RandomGenerator {
public:
    ~RandomGenerator();

    static RandomGenerator* getInstance();

    int generateInt(int dim);
    float generateFloat(float maxn  = 1.f);
    Eigen::VectorXf generateState(int dim, float maxn = 1.f);
    Eigen::VectorXf generateAction(int dim, const Eigen::VectorXf & maxn);
    Eigen::MatrixXf generateTerrain(int w, int h, int maxn = 1);

    Eigen::VectorXf generateVector(int dim, float maxn = 1.f, bool bothside = true);
private:
    RandomGenerator();

    static RandomGenerator* instance;
    std::default_random_engine generator;
};

#endif
