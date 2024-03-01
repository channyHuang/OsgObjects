#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include "commonFunc/FastNoiseLite.h"
#include "commonMath/box.h"
#include "voxels/voxelBuffer.h"
#include <memory>

class TerrainGenerator
{
public:
    enum TerrainType {
        Terrain_Default = 0,
        Terrain_Flat,
        Terrain_Moutain
    };

    TerrainGenerator();
    ~TerrainGenerator();

    void generateTerrain(VoxelBuffer &buffer, TerrainType eTerrainType = Terrain_Default);

private:
    Box bRange;
    FastNoiseLite noise_lite;
};

#endif
