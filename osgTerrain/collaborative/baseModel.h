#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <vector>

#include "Eigen/Core"
#include <iostream>
#include "common.h"

// Layer NN

enum E_Activation {
    Linear = 0,
    Tanh,
    Sigmoid,
    Relu,
    Sgn
};

// NN Base Model for Actor and Critic

class BaseModel {
public:
    BaseModel(uint32_t numOfLayer = 3,
              const std::vector<uint32_t>& layerWidth = {1, 1, 1},
              float learningRate = 0.5f,
              const std::vector<E_Activation>& activateType = {},
              uint32_t batchSize = 1) {
        assert(numOfLayer >= 3);
        assert(layerWidth.size() == numOfLayer);

        nBatchSize = batchSize;
        fLearningRate = learningRate;
        nNumOfLayers = numOfLayer;
        vLayerWidth = layerWidth;
        vMatWeights.resize(nNumOfLayers - 1);
        for (uint32_t i = 0; i < nNumOfLayers - 1; ++i) {
            vMatWeights[i] = Eigen::MatrixXf(vLayerWidth[i + 1], vLayerWidth[i]);
            vMatWeights[i].fill(1.f);
        }
        vBias.resize(nNumOfLayers - 1, 0);
        vHiddenValues.resize(nNumOfLayers);
        vErrors.resize(nNumOfLayers);
        vActivateType.resize(nNumOfLayers, Relu);
        for (uint32_t i = 0; i < nNumOfLayers; ++i) {
            if (i < activateType.size()) {
                vActivateType[i] = activateType[i];
            }
            vHiddenValues[i].resize(vLayerWidth[i]);
            vErrors[i].resize(vLayerWidth[i]);
        }
    };

    virtual ~BaseModel() {}

    // predict action base on state
    virtual Eigen::VectorXf predict(const Eigen::VectorXf &input) {
        if (input.size() != vLayerWidth[0]) {
            std::cout << "BaseModel predict error: size not fit" << std::endl;
            return Eigen::VectorXf(vLayerWidth[nNumOfLayers - 1]);
        }
        vHiddenValues[0] = input;
        for (uint32_t i = 0; i < nNumOfLayers - 1; ++i) {
            Eigen::VectorXf bias = Eigen::VectorXf(vLayerWidth[i + 1]).setOnes() * vBias[i];
            vHiddenValues[i + 1] = vMatWeights[i] * vHiddenValues[i] + bias;
        }
        softmax(vHiddenValues[nNumOfLayers - 1], vMatWeights[nNumOfLayers - 2]);
        return vHiddenValues[nNumOfLayers - 1];
    }

    // train
    virtual void train(const Eigen::VectorXf& input, const Eigen::VectorXf& output) {
        if (input.size() != vLayerWidth[0] || output.size() != vLayerWidth[nNumOfLayers - 1]) {
            return;
        }
        predict(input); 
        backTrace(output);
    }

    // back trace
    virtual void backTrace(const Eigen::VectorXf& output) {
        Eigen::VectorXf backValue = output;
        vErrors[nNumOfLayers - 1].fill(0.f);
        for (uint32_t j = 0; j < vLayerWidth[nNumOfLayers - 1]; ++j) {
            vErrors[nNumOfLayers - 1][j] = fabs(backValue[j] - vHiddenValues[nNumOfLayers - 1](j));
        }
        for (int i = nNumOfLayers - 2; i >= 0; --i) {
            for (uint32_t j = 0; j < vLayerWidth[i]; ++j) {
                float weightSum = vMatWeights[i].col(j).sum();
                if (isZero(weightSum)) weightSum = 1.f;
                float invWeightSum = 1.f / weightSum;
                vErrors[i](j) = vMatWeights[i].col(j).dot(vErrors[i + 1]) * invWeightSum; 
                vMatWeights[i].col(j) += fLearningRate * vErrors[i][j] * vMatWeights[i].col(j) * invWeightSum;
                vBias[i] += fLearningRate * vErrors[i][j];
            }
        }
    }

    // others
    virtual Eigen::VectorXf computeGradient() { return Eigen::VectorXf(); }
    std::vector<Eigen::MatrixXf> getWeights() { return vMatWeights; }
    std::vector<Eigen::MatrixXf>& getAndChangeWeights() { return vMatWeights; }

private:
    float activate(float v, E_Activation activateType) {
        switch(activateType) {
        case Relu:
            return 1.f / (1 + exp(-v));
        case Sgn:
            return (v <= 0 ? 0 : 1);
        case Sigmoid:
            return (v <= 0 ? 0 : v);
        case Tanh:
            return tan(v);
        case Linear:
        default:
            break;
        }
        return v;
    }

    void softmax(Eigen::VectorXf& v, const Eigen::MatrixXf& weight) {
        float sum = v.sum();
        if (isZero(sum)) sum = 1.f;
        float invsum = 1.f / sum;
        for (uint32_t i = 0; i < v.size(); ++i) {
            v[i] *= invsum;
        }
        return;

        if (v.size() != weight.rows()) {
            std::cout << "BaseModel softmax error: size not fit" << std::endl;
            return;
        }

        float totalSum = weight.sum();
        if (isZero(totalSum)) totalSum = 1.f;
        for (uint32_t i = 0; i < v.size(); ++i) {
            float sum = weight.col(i).sum();
            v[i] *= sum / totalSum;
        }
    }

    // network setting param
    uint32_t nBatchSize = 1;
    float fLearningRate = 0.5f;
    uint32_t nNumOfLayers = 1;
    std::vector<uint32_t> vLayerWidth;
    std::vector<E_Activation> vActivateType;

    // network fitting param
    std::vector<Eigen::MatrixXf> vMatWeights;
    std::vector<float> vBias = {};

    // network compute value
    std::vector<Eigen::VectorXf> vHiddenValues;
    std::vector<Eigen::VectorXf> vErrors;
    Eigen::VectorXf gradient = Eigen::VectorXf();
};

#endif
