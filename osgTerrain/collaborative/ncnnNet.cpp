#include "ncnnNet.h"

NcnnNet::NcnnNet(uint32_t numOfLayer,
             const std::vector<uint32_t>& layerWidth,
             float learningRate,
             const std::vector<E_Activation>& activateType,
             uint32_t batchSize)
    : BaseModel(numOfLayer, layerWidth, learningRate, activateType, batchSize) {
    if (!loadNet("E:/projects/test/actor.param")) {
        std::cout << "load net failed" << std::endl;
    }
}

NcnnNet::~NcnnNet() {}

Eigen::VectorXf NcnnNet::eval(const Eigen::VectorXf& state) {
    ncnn::Mat in(1, state.size(), 1, (void*)state.data());
    ncnn::Mat out;
    ncnn::Extractor extractor = net.create_extractor();
    int res = extractor.input("data", in);
    if (res < 0) {
        std::cout << "input error" << std::endl;
    }
    std::cout << "start to extract " << std::endl;
    res = extractor.extract("prob", out);
    if (res < 0) {
        std::cout << "output error" << std::endl;
    }
    Eigen::VectorXf y(out.h);
    memcpy((void*)y.data(), out.data, out.w * out.h * sizeof(float));
    return y;
}

bool NcnnNet::loadNet(const std::string& netFile) {
    int res = net.load_param(netFile.c_str());
    if (res) return false;
    ncnn::DataReader dr;
    res = net.load_model(dr);
    return (res == -1 ? false : true);
}
