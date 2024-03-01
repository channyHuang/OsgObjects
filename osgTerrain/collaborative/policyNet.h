#ifndef POLICYNET_H
#define POLICYNET_H

#include "baseModel.h"
#include "dataStruct.h"

enum OutputType {
    QValue,
    VValue,
    Prob,
    Action
};

class PolicyNet : public BaseModel {
public:
    PolicyNet(uint32_t numOfLayer = 3,
        const std::vector<uint32_t>& layerWidth = { 1, 1, 1 });
    virtual ~PolicyNet();

    Eigen::VectorXf getAction(const Eigen::VectorXf& state);

    void trainActor(const Eigen::VectorXf& state, const Eigen::VectorXf& action);

    float getValue(const Eigen::VectorXf& state, const Eigen::VectorXf& action);

    void trainCritic(const MyTuples& tuple);

private:
    OutputType eOutputType = Action;
};

#endif
