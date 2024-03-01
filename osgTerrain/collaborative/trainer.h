#ifndef TRAINER_H
#define TRAINER_H

#include "policyNet.h"
#include "dataStruct.h"

class Trainer {
public:
    Trainer(uint32_t stateDim, uint32_t actionDim);

    void init();

    void addTuple(const MyTuples& tuple);
    MyTuples getTuple(uint32_t idx);

    Eigen::VectorXf getAction(const Eigen::VectorXf& state, int policy = 0);
    float calcReward(const MyTuples& tuple);
    void train(MyTuples& tuple, bool isActor);
    float getQValue(MyTuples& tuple);
    float getQNextValue(MyTuples& tuple);

    float getTerrainHeight(const Eigen::VectorXf& pos) const;

    void setTerrainDim(int dim) { nTerrainDim = dim; }
    void setTerrainHeight(int h) { nMaxTerrainHeight = h; }
private:
    std::vector<MyTuples> vTupleBuffer;
    int nTupleEndPos = 0;
    uint32_t nNumOfTuple = 0;
    uint32_t nNumOfTupleBuffer = 1;

    uint32_t nDimOfAction = 2;
    Eigen::VectorXf vMaxAction;

    PolicyNet actor;
    std::vector<PolicyNet> critics;

    int nTerrainDim = 100;
    int nMaxTerrainHeight = 10;
    Eigen::MatrixXf mTerrain;

    Eigen::VectorXf vRewardWeights;
    std::vector<float> fGama = {0.5f};
};

#endif
