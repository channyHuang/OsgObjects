#include "trainTestCol.h"

#include <iostream>

TrainTestCol::TrainTestCol() {

}

float slamp(float in, float min, float max) {
    if (in < min) return min;
    if (in >= max) return max - 1;
    return in;
}

void TrainTestCol::test(Trainer& trainer) {
    uint32_t nIterateTimes = 10000;
    // test
    Eigen::VectorXf pos = RandomGenerator::getInstance()->generateState(2, nTerrainDim);
    Eigen::VectorXf targetPos = RandomGenerator::getInstance()->generateState(2, nTerrainDim);

    Eigen::VectorXf state(nStateDim);
    state.segment(0, 2) = pos;
    state.segment(2, 2) = RandomGenerator::getInstance()->generateState(2, nTerrainDim);
    state(4) = trainer.getTerrainHeight(pos);

    Eigen::VectorXf action;
    uint32_t i = 0;
    output2File("mappo.txt", state);
    while (true) {
        // get action by policy
        action = trainer.getAction(state, false);
        state.segment(0, 2) += action;
        targetPos = RandomGenerator::getInstance()->generateVector(2);
        state.segment(2, 2) += targetPos;
        state(0) = slamp(state(0), 0, 100);
        state(1) = slamp(state(1), 0, 100);
        state(2) = slamp(state(2), 0, 100);
        state(3) = slamp(state(3), 0, 100);
        state(4) = trainer.getTerrainHeight(state.segment(0, 2));
        output2File("mappo.txt", state);
        if (i++ >= nIterateTimes) break;
    }
}

void TrainTestCol::optimize(Trainer& trainer) {
    Eigen::VectorXf pos = RandomGenerator::getInstance()->generateState(2, nTerrainDim);
    Eigen::VectorXf targetPos = RandomGenerator::getInstance()->generateState(2, nTerrainDim);

    uint32_t nIterateTimes = 1000;
    float lastDist = std::sqrt((pos.x() - targetPos.x()) * (pos.x() - targetPos.x()) + (pos.y() - targetPos.y()) * (pos.y() - targetPos.y()));;

    Eigen::VectorXf state(nStateDim);
    state.segment(0, 2) = pos;
    state.segment(2, 2) = targetPos;
    state(4) = trainer.getTerrainHeight(pos);

    std::vector< Eigen::VectorXf> dirs;
    Eigen::VectorXf dir(2);
    dir(0) = -1;
    dir(1) = -1;
    dirs.push_back(dir);
    dir(0) = -1;
    dir(1) = 1;
    dirs.push_back(dir);
    dir(0) = 1;
    dir(1) = -1;
    dirs.push_back(dir);
    dir(0) = 1;
    dir(1) = 1;
    dirs.push_back(dir);

    for (uint32_t i = 0; i < nIterateTimes; ++i) {
        Eigen::VectorXf targetMove = RandomGenerator::getInstance()->generateVector(2);
        {
            int nextIdx = 0;
            float score = 0;
            float nextdist = 0;
            for (int j = 0; j < dirs.size(); ++j) {
                Eigen::VectorXf nextPos = pos + dirs[j];
                float dist = std::sqrt((nextPos.x() - targetPos.x()) * (nextPos.x() - targetPos.x()) + (nextPos.y() - targetPos.y()) * (nextPos.y() - targetPos.y()));
                float diffHeight = trainer.getTerrainHeight(pos) - trainer.getTerrainHeight(nextPos);

                float newScore = (lastDist - dist - diffHeight);
                if (newScore > score) {
                    nextIdx = j;
                    score = newScore;
                    nextdist = dist;
                }
            }

            Eigen::VectorXf newState = state;
            newState.segment(0, 2) += dirs[nextIdx];
            newState.segment(2, 2) += targetMove;

            pos += dirs[nextIdx];
            targetPos += targetMove;
            output2File("mappo.txt", newState);
            state = newState;
            lastDist = nextdist;
        }
    }
}

void TrainTestCol::train(Trainer& trainer, int nNumOfPlatform) {
    std::vector< Eigen::VectorXf> vPlatformPos(nNumOfPlatform);

    for (int i = 0; i < nNumOfPlatform; ++i) {
        vPlatformPos[i] = RandomGenerator::getInstance()->generateState(2, nTerrainDim);
    }
}

void TrainTestCol::train(Trainer& trainer) {
    Eigen::VectorXf pos = RandomGenerator::getInstance()->generateState(2, nTerrainDim);
    Eigen::VectorXf targetPos = RandomGenerator::getInstance()->generateState(2, nTerrainDim);

    Eigen::VectorXf state(nStateDim);
    state.segment(0, 2) = pos;
    state.segment(2, 2) = targetPos;
    state(4) = trainer.getTerrainHeight(pos);

    // train
    int getActionByPolicy = 1;
    uint32_t nIterateTimes = 100000;

    for (uint32_t i = 0; i < nIterateTimes; ++i) {
        // get action by policy
        Eigen::VectorXf action = trainer.getAction(state, getActionByPolicy);
        getActionByPolicy = 1;

        // apply action, calc reward
        Eigen::VectorXf stateNext = state;
        stateNext.segment(0, 2) += action;
        targetPos = RandomGenerator::getInstance()->generateVector(2);
        stateNext.segment(2, 2) += targetPos;
        stateNext(0) = slamp(stateNext(0), 0, 100);
        stateNext(1) = slamp(stateNext(1), 0, 100);
        stateNext(2) = slamp(stateNext(2), 0, 100);
        stateNext(3) = slamp(stateNext(3), 0, 100);
        stateNext(4) = trainer.getTerrainHeight(stateNext.segment(0, 2));

        float reward = 0;
        // add tuple buffer
        MyTuples tuple(i, reward, state, action, stateNext);
        reward = trainer.calcReward(tuple);
        if (reward < -1000.f) {
            getActionByPolicy = 0;
        }
        tuple.fReward = reward;
        trainer.addTuple(tuple);
        // feedback network
        trainer.train(tuple, false);
        trainer.train(tuple, true);     

        state = stateNext;
    }
}

int TrainTestCol::colliborate()
{
    Trainer trainer(nStateDim, 2);
    trainer.setTerrainDim(nTerrainDim);
    trainer.setTerrainHeight(nMaxTerrainHeight);
    trainer.init();

    std::cout << "start training" << std::endl;
    train(trainer);
    std::cout << "start testing" << std::endl;
    test(trainer);
    std::cout << "end" << std::endl;
    return 0;
}
