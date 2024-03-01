#include "policyNet.h"

PolicyNet::PolicyNet(uint32_t numOfLayer, const std::vector<uint32_t>& layerWidth)
    : BaseModel(numOfLayer, layerWidth)
{}

PolicyNet::~PolicyNet() {}

Eigen::VectorXf PolicyNet::getAction(const Eigen::VectorXf &state) {
    Eigen::VectorXf output = BaseModel::predict(state);
    // add noise
    //output += RandomGenerator::getInstance()->generateAction(output.size(), Eigen::VectorXf(output.size()).setOnes());

    return output;
}

void PolicyNet::trainActor(const Eigen::VectorXf& state, const Eigen::VectorXf& action) {
    BaseModel::train(state, action);
}

float PolicyNet::getValue(const Eigen::VectorXf& state, const Eigen::VectorXf& action) {
    Eigen::VectorXf output = BaseModel::predict(state);
    float value = output(0);
    return value;
}

void PolicyNet::trainCritic(const MyTuples& tuple) {
    Eigen::VectorXf y(1);
    y(0) = tuple.qValue;
    BaseModel::backTrace(y);
}
