#include "voxelBrush.h"

VoxelBrush::VoxelBrush(TerrainManager* manager, VoxelMap* map) {
    _terrainManager = manager;
    _map = map;
}
VoxelBrush::~VoxelBrush() {}

int VoxelBrush::get_voxel(const Vector3i& pos) {
    return _map->get_voxel(pos, _channel);
}

float VoxelBrush::get_voxel_f(const Vector3i& pos) {
    return _map->get_voxel_f(pos, _channel);
}

int VoxelBrush::get_voxel(const Vector3i& pos, int channel) {
    return _map->get_voxel(pos, channel);
}

float VoxelBrush::get_voxel_f(const Vector3i& pos, int channel) {
    return _map->get_voxel_f(pos, channel);
}

void VoxelBrush::set_voxel(const Vector3i& pos, int v) {
    _map->set_voxel(v, pos, _channel);
    _post_edit(pos);
}

void VoxelBrush::set_voxel_f(const Vector3i& pos, float v) {
    _map->set_voxel_f(v, pos, _channel);
    _post_edit(pos);
}

void VoxelBrush::set_voxel(const Vector3i& pos, int v, int channel) {
    _map->set_voxel(v, pos, channel);
    _post_edit(pos);
}

void VoxelBrush::set_voxel_f(const Vector3i& pos, float v, int channel) {
     _map->set_voxel_f(v, pos, channel);
     _post_edit(pos);
}

void VoxelBrush::set_voxel_info(const Vector3i& pos, float v, int material) {
    _map->set_voxel_f(v, pos, VoxelBuffer::CHANNEL_SDF);
    _map->set_voxel(material, pos, VoxelBuffer::CHANNEL_TYPE);
    _post_edit(pos);
}

void VoxelBrush::_post_edit(const Boxi& box) {
    _terrainManager->make_area_dirty(box);
}

void VoxelBrush::_post_edit(const Vector3i& pos) {
    _terrainManager->make_voxel_dirty(pos);
}
