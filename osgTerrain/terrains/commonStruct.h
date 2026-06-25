#pragma once

#include <array>
#include <memory>

#include "voxels/voxelBuffer.h"
#include "commonMath/vector2.h"

struct Arrays {
    bool isWater;
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<uint32_t> indices;
    std::vector<uint32_t> materials;
    std::array<std::vector<uint32_t>, /*Cube::SIDE_COUNT*/6> transition_surfaces;
    std::vector<uint32_t> indices_water;
    std::vector<Vector2> uvs;

    bool empty() {
        return (positions.size() <= 0 || indices.size() <= 0);
    }
};

struct MeshInput {
    int lod;
    Vector3i position;
    //const VoxelBuffer &voxels;
    std::shared_ptr<VoxelBuffer> voxels;
};

struct MeshOutput {
    std::vector<Arrays> surfaces;
};

    struct InputBlock {
        std::shared_ptr<VoxelBuffer> voxels;
        Vector3i position;
        uint8_t lod = 0;
        bool can_be_discarded = true;
        float sort_heuristic = 0;
    };

    struct OutputBlock {
        MeshOutput blocky_surfaces;
        MeshOutput smooth_surfaces;
        Vector3i position;
        uint8_t lod = 0;
        bool drop_hint = false;
    };

    struct Input {
        std::vector<InputBlock> blocks;

        bool is_empty() const {
            return blocks.empty();
        }
    };

    struct Output {
        std::vector<OutputBlock> blocks;
    };


