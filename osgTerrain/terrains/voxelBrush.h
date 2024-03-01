#ifndef VOXELBRUSH_H
#define VOXELBRUSH_H

#include "commonMath/boxi.h"
#include "voxelmap.h"
#include "terrainManager.h"

class VoxelBrush {
public:
    VoxelBrush(TerrainManager* manager, VoxelMap* map);
    ~VoxelBrush();

    int get_voxel(const Vector3i& pos);
    float get_voxel_f(const Vector3i& pos);
    int get_voxel(const Vector3i& pos, int channel);
    float get_voxel_f(const Vector3i& pos, int channel);

    void set_voxel(const Vector3i& pos, int v);
    void set_voxel_f(const Vector3i& pos, float v);
    void set_voxel(const Vector3i& pos, int v, int channel);
    void set_voxel_f(const Vector3i& pos, float v, int channel);

    void set_voxel_info(const Vector3i& pos, float v, int material);

private:
    void _post_edit(const Boxi& box);
    void _post_edit(const Vector3i& pos);

    int _channel = VoxelBuffer::CHANNEL_SDF;

    VoxelMap* _map = nullptr;
    TerrainManager* _terrainManager = nullptr;
};

#endif
