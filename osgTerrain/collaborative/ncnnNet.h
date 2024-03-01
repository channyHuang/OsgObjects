#ifndef NCNNNET_H
#define NCNNNET_H

#include <iostream>

#include "baseModel.h"
#include "dataStruct.h"

#include <ncnn/net.h>
#include <ncnn/datareader.h>

class NcnnNet : public BaseModel {
public:
    NcnnNet(uint32_t numOfLayer = 3,
          const std::vector<uint32_t>& layerWidth = {1},
          float learningRate = 0.5f,
          const std::vector<E_Activation>& activateType = {Relu},
          uint32_t batchSize = 1);
    virtual ~NcnnNet();

    // predict
    Eigen::VectorXf eval(const Eigen::VectorXf& state);

    bool loadNet(const std::string& netFile);

private:
    ncnn::Net net;
};

#endif
