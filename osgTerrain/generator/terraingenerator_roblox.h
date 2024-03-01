#ifndef TERRAINGENERATOR_ROBLOX_H
#define TERRAINGENERATOR_ROBLOX_H

#include <vector>
#include <functional>
#include <chrono>
#include <fstream>
#include <queue>

#include "commonMath/vector3.h"
#include "commonMath/vector3i.h"
//#include "terrains/voxeltoolterrain.h"
#include "voxels/common_enum.h"
#include "commonMath/funcs.h"
#include "terrains/voxelBrush.h"

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


class TerrainGenerator_Roblox
{
public:
    struct BiomesParam
    {
        int biomes_be_checked = 3; // checked types of biomes
        bool use_biomes = true; // if use auto biomes or just generate flat
        int biome_size = 50; // area of each biome
        bool generate_caves = false;
        std::string seed = "618033988"; // biome seed
    };

    enum TerrainBiomes {
        Water = 0,
        Marsh,
        Plains,
        Hills,
        Dunes,
        Canyons,
        Mountains,
        Lavaflow,
        Arctic,
        TerrainBiomesCount
    };

    TerrainGenerator_Roblox();
    ~TerrainGenerator_Roblox();

    static std::shared_ptr<TerrainGenerator_Roblox> getInstance();

    void setRange(const Vector3& vStart, const Vector3i& vSize) {
        _range.vStart = vStart - vSize.to_vec3() * 0.5f;
        _range.vSize = vSize;
        _range.vBox = Box(_range.vStart, _range.vStart + _range.vSize.to_vec3());
    }
    //biomes be checked, nbiomes_be_checked recode Biomes[i] be checked if the bit in i of nbiomes_be_checked is 1
    void setBiomesBeChecked(int nbiomes_be_checked) {
        if (nbiomes_be_checked_ == nbiomes_be_checked) return;
        vbiomes_.clear();
        for (int i = 0; i < TerrainBiomesCount; ++i) {
            if (nbiomes_be_checked & (1 << i)) {
                vbiomes_.push_back(static_cast<TerrainBiomes>(i));
            }
        }
        nbiomes_be_checked_ = nbiomes_be_checked;
    }
    //biome size
    void setBiomeSize(int nbiome_size = 20) { nbiome_size_ = nbiome_size; }
    int getBiomeSize() { return nbiome_size_; }
    //include caves ?
    void setIncludeCaves(bool binclude_caves = true) { binclude_caves_ = binclude_caves; }
    bool getIncludeCaves() { return binclude_caves_; }
    //seed setting
    void setTerrainGeneratorSeed(std::string seedNumberStr);
    //generate terrain with biomes
    //void genTotallyFlat(VoxelToolTerrain* pVoxelTool, int max_lod = 0);
    //void generateTerrainByBiomes(VoxelToolTerrain* pVoxelTool, BiomesParam& biomeParams, int max_lod = 0);
    void genTotallyFlat(VoxelBrush* pVoxelTool, int max_lod = 0);
    void generateTerrainByBiomes(VoxelBrush* pVoxelTool, BiomesParam& biomeParams, int max_lod = 0);

    struct PointDistNoiseInfo {
        float fdist_, fbiome_noise_;
        PointDistNoiseInfo(float _dist, float _biomeNoise) : fdist_(_dist), fbiome_noise_(_biomeNoise) {}
    };
    // single voxel info in terrain generation
    struct PointVoxelInfo {
        float foccupancy_, fweight_;
        MaterialType eSurfaceMaterial_, eFillMaterial_;
        PointVoxelInfo(float _fweight = 0) : fweight_(_fweight) {}
    };

public:
    struct {
        Vector3i vSize = Vector3i(100);
        Vector3 vStart = Vector3(0);
        Box vBox = Box(Vector3(-10.f), Vector3(10.f));
    } _range;
    uint32_t nbiomes_be_checked_ = 0;
    float progress = 0.f;

private:
    //wrap data
    float ridgedFilter(float value) {
        return (value < .5f ? value * 2.f : 2.f - value * 2.f);
    }
    float thresholdFilter(float value, float bottom, float size) {
        if (value <= bottom) return 0;
        else if (value >= bottom + size) return 1;
        return (value - bottom) / size;
    }
    float ridgedFlippedFilter(float value) {
        return (value < .5f ? 1.f - value * 2.f : value * 2.f - 1.f);
    }

    float mountainsOperation(int x, int y, int z, int i) {
        return ridgedFilter(getPerlin(x, y, z, 100 + i, (1 / i) * 160));
    }

    float fractalize(std::function<float(int, int, int, int)> operation,
        int x, int y, int z,
        int operationCount = 3, float scale = .5f, float offset = 0, float gain = 1) {
        float totalValue = 0, totalScale = 0;
        for (unsigned int i = 0; i < operationCount; ++i) {
            float thisScale = std::pow(scale, i);
            totalScale += thisScale;
            totalValue += (offset + gain * operation(x, y, z, i + 1)) * thisScale;
        }
        return totalValue / totalScale;
    }
    //unit test, read voxel data from file, data format (for each line): x y z sdf material
    //void readDataFromFiles(std::string sDataFileName, VoxelToolTerrain* pVoxelTool);

    void initData();
    float getNoise(int x, int y, int z, int seed1 = 7);
    float getNoise(int x, int z);
    float getPerlin(float x, float y, float z, int seed = 0, float scale = 1.f, bool raw = false);
    PointVoxelInfo findBiomeInfo(TerrainBiomes choiceBiome, int x, int y, int z, float verticalGradientTurbulence);
    float findBiomeTransitionValue(TerrainBiomes biome, float weight, float value, float averageValue);

    float translateSdfAndOccupancy(float value, bool bSdf2Occupancy) {
        return (bSdf2Occupancy ? (value > 0 ? 0 : -value) : (value <= 0 ? 1 : -value));
    }

private:
    

    static std::shared_ptr<TerrainGenerator_Roblox> pInstance_;
    long long nmaster_seed_ = 618033988;
    bool binclude_caves_ = false;
    int nbiome_size_ = 100;
    float fwater_level_ = .48f;
    float fsurface_thickness_ = .018f;

    
    std::vector<TerrainBiomes> vbiomes_;
    std::vector<float> vtheseed_;
    int ntheseed_size_ = 0;
    //in roblox, resolution = 4, while in blockman, we use resolution = 1
    float fresolution_ = 1.f;
    float fbiome_blend_percent_ = .25f;
    float fbiome_blend_percent_inverse_ = 1.f - fbiome_blend_percent_;
    TerrainSimplePerlin cperlin_;
    const float foccupancy_step_ = 1.f / 254.f;

    std::queue<Vector3> m_quTreeRoot;
};



#endif // TERRAINGENERATOR_ROBLOX_H
