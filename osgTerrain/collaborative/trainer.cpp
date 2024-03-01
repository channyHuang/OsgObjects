#include "trainer.h"


#include "commonMath/noise.h"

class TerrainSimplePerlin {
public:
    void load() {
        if (permutation_.size() < nsize_) {
            return;
        }
        p_.resize(nsize_ + nrand_max_, 0);
        for (unsigned int i = 1; i <= nsize_; i++) {
            p_[i] = permutation_[i - 1];
            p_[i + nrand_max_ - 1] = p_[i];
        }
    }

    float fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }

    float grad(float hash, float x, float y, float z) {
        int h = BitAND(hash, 15);
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : ((h == 12 || h == 14) ? x : z);
        return ((h && 1) == 0 ? u : -u) + ((h && 2) == 0 ? v : -v);
    }

    int BitAND(int a, int b) {
        int p = 1, c = 0;
        while (a > 0 && b > 0) {
            int ra = a & 1, rb = b & 1;
            if (ra + rb > 1) c = c + p;
            a = (a - ra) >> 1;
            b = (b - rb) >> 1;
            p <<= 1;
        }
        return c;
    }

    float noise(float x, float y, float z) {
        int X = BitAND(std::floor(x), 255) + 1;
        int Y = BitAND(std::floor(y), 255) + 1;
        int Z = BitAND(std::floor(z), 255) + 1;

        x = x - std::floor(x);
        y = y - std::floor(y);
        z = z - std::floor(z);
        float u = fade(x);
        float v = fade(y);
        float w = fade(z);
        float A = p_[X] + Y;
        float AA = p_[A] + Z;
        float AB = p_[A + 1] + Z;
        float B = p_[X + 1] + Y;
        float BA = p_[B] + Z;
        float BB = p_[B + 1] + Z;

        return lerp(w, lerp(v, lerp(u, grad(p_[AA], x, y, z),
            grad(p_[BA], x - 1, y, z)),
            lerp(u, grad(p_[AB], x, y - 1, z),
                grad(p_[BB], x - 1, y - 1, z))),
            lerp(v, lerp(u, grad(p_[AA + 1], x, y, z - 1),
                grad(p_[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad(p_[AB + 1], x, y - 1, z - 1),
                    grad(p_[BB + 1], x - 1, y - 1, z - 1))));
    }

private:
    int nsize_ = 256, nrand_max_ = 256;
    std::vector<int> p_, permutation_ = { 151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };
};

Trainer::Trainer(uint32_t stateDim, uint32_t actionDim) {
    vTupleBuffer.resize(nNumOfTupleBuffer);

    nDimOfAction = actionDim;

    vMaxAction.resize(nDimOfAction);
    vMaxAction.fill(1.f);

    vRewardWeights.resize(3);
    vRewardWeights(0) = 0.1f;
    vRewardWeights(1) = 0.7f;
    vRewardWeights(2) = 0.2f;

    actor = PolicyNet(3, {stateDim, 8, actionDim});
    float sum = (1 + nNumOfTupleBuffer) * nNumOfTupleBuffer * 0.5f;
    if (isZero(sum)) sum = 1;
    float invsum = 1.f / sum;
    critics.resize(nNumOfTupleBuffer);
    for (uint32_t i = 0; i < nNumOfTupleBuffer; ++i) {
        critics[i] = PolicyNet(3, { stateDim, 8, 1 });
        fGama[i] = (i + 1) * invsum;
	}
}

void Trainer::init() {
    TerrainSimplePerlin cperlin_;
    cperlin_.load();

    fast_noise_lite::FastNoiseLite noiseLite;
    noiseLite.SetNoiseType(fast_noise_lite::FastNoiseLite::NoiseType_PerlinNew);
    noiseLite.SetFractalType(fast_noise_lite::FastNoiseLite::FractalType_FBm);

    mTerrain = RandomGenerator::getInstance()->generateTerrain(nTerrainDim, nTerrainDim, nMaxTerrainHeight);
    std::ofstream ofs("terrain.txt");

    ofs << nTerrainDim << std::endl;
    for (int i = 0; i < nTerrainDim; ++i) {
        for (int j = 0; j < nTerrainDim; ++j) {
            mTerrain(i, j) = noiseLite.GetNoise<float>(i, j) * nMaxTerrainHeight;
            ofs << mTerrain(i, j) << " ";
        }
        ofs << std::endl;
    }
    ofs.close();
}

MyTuples Trainer::getTuple(uint32_t idx) {
    return vTupleBuffer[idx];
}

void Trainer::addTuple(const MyTuples& tuple) {
    vTupleBuffer[nTupleEndPos] = tuple;
    nTupleEndPos = (nTupleEndPos + 1) % nNumOfTupleBuffer;

    if (nNumOfTuple < nNumOfTupleBuffer) {
        nNumOfTuple++;
    }
}

// policy: 0: random; 1: prob random; 2: policy
Eigen::VectorXf Trainer::getAction(const Eigen::VectorXf& state, int policy) {
    const float epsilon = 0.3f;
    float lambda = RandomGenerator::getInstance()->generateFloat();
    if (policy == 0 || (policy ==1 && lambda < epsilon)) {
        return RandomGenerator::getInstance()->generateAction(nDimOfAction, vMaxAction);
    }
    return actor.getAction(state);
}

float Trainer::calcReward(const MyTuples& tuple) {
    if (!isValidPos(tuple.stateNext.segment(0, 2), nTerrainDim) || !isValidPos(tuple.stateNext.segment(2, 2), nTerrainDim)) return -10000.f;

    Eigen::VectorXf err(3);
    const float gamma = 0.9f;
    Eigen::VectorXf diffAction = (vMaxAction - tuple.action);

    err(0) = std::exp(-gamma * diffAction.norm());
    err(1) = -(tuple.stateNext.segment(2, 2) - tuple.stateNext.segment(0, 2)).norm();
    err(2) = mTerrain((int)tuple.stateNext(0), (int)tuple.stateNext(1)) - mTerrain((int)tuple.state(0), (int)tuple.state(1));

    float reward = vRewardWeights.transpose() * err;
    return reward;
}

void Trainer::train(MyTuples& tuple, bool isActor) {
    if (isActor) {
        float err = (tuple.qNextValue - tuple.qValue);
        actor.train(tuple.state, tuple.action + Eigen::VectorXf::Ones(nDimOfAction) * err);
        return;
    }
    Eigen::VectorXf reward;
    for (uint32_t i = 0; i < critics.size(); ++i) {
        float y = 0.f;
        for (uint32_t j = 0; j < nNumOfTuple; ++j) {
            MyTuples tmpTuple = getTuple(j);
            float reward = calcReward(tmpTuple);

            y += reward + fGama[i] * tmpTuple.qValue;
        }
        y /= critics.size();
        tuple.qValue = y;
        critics[i].trainCritic(tuple);
    }
}

float Trainer::getQValue(MyTuples& tuple) {
    for (uint32_t i = 0; i < critics.size(); ++i) {
        tuple.qValue += critics[i].getValue(tuple.state, tuple.action);
    }
    tuple.qValue /= critics.size();
    return tuple.qValue;
}

float Trainer::getQNextValue(MyTuples& tuple) {
    for (uint32_t i = 0; i < critics.size(); ++i) {
        tuple.qNextValue += critics[i].getValue(tuple.stateNext, tuple.action);
    }
    tuple.qNextValue /= critics.size();
    return tuple.qNextValue;
}

float Trainer::getTerrainHeight(const Eigen::VectorXf& pos) const {
    if ((int)pos(0) < 0 || (int)pos(0) >= nTerrainDim) return nMaxTerrainHeight;
    if ((int)pos(1) < 0 || (int)pos(1) >= nTerrainDim) return nMaxTerrainHeight;
    return mTerrain((int)pos(0), (int)pos(1));
}
