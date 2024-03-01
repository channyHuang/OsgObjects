#include "voxelblock.h"


// Faster version of Mesh::create_trimesh_shape()
// See https://github.com/Zylann/godot_voxel/issues/54
//
// Helper
VoxelBlock *VoxelBlock::create(Vector3i bpos, VoxelBuffer* buffer, unsigned int size, unsigned int p_lod_index) {
    const int bs = size;

    VoxelBlock *block = new(VoxelBlock);
    block->position = bpos;
    block->lod_index = p_lod_index;
    block->_position_in_voxels = bpos * (size << p_lod_index);
    block->voxels = buffer;

#ifdef VOXEL_DEBUG_LOD_MATERIALS
    Ref<SpatialMaterial> debug_material;
    debug_material.instance();
    int checker = (bpos.x + bpos.y + bpos.z) & 1;
    Color debug_color = Color(0.8, 0.4, 0.8).linear_interpolate(Color(0.0, 0.0, 0.5), static_cast<float>(p_lod_index) / 8.f);
    debug_color = debug_color.lightened(checker * 0.1f);
    debug_material->set_albedo(debug_color);
    block->_debug_material = debug_material;

    Ref<SpatialMaterial> debug_transition_material;
    debug_transition_material.instance();
    debug_transition_material->set_albedo(Color(1, 1, 0));
    block->_debug_transition_material = debug_transition_material;
#endif

    return block;
}

VoxelBlock::VoxelBlock() {
}

VoxelBlock::~VoxelBlock() {
}


void VoxelBlock::set_mesh_state(MeshState ms) {
    _mesh_state = ms;
}

VoxelBlock::MeshState VoxelBlock::get_mesh_state() const {
    return _mesh_state;
}

void VoxelBlock::set_visible(bool visible) {
    if (_visible == visible) {
        return;
    }
    _visible = visible;
    _set_visible(_visible && _parent_visible);
}

bool VoxelBlock::is_visible() const {
    return _visible;
}

void VoxelBlock::_set_visible(bool visible) {/*
    if (_mesh_instance.is_valid()) {
        set_mesh_instance_visible(_mesh_instance, visible);
    }
    for (unsigned int dir = 0; dir < _transition_mesh_instances.size(); ++dir) {
        DirectMeshInstance &mi = _transition_mesh_instances[dir];
        if (mi.is_valid()) {
            set_mesh_instance_visible(mi, visible && _is_transition_visible(dir));
        }
    }
    if (_static_body.is_valid()) {
        _static_body.set_shape_enabled(0, visible);
    }*/
}

void VoxelBlock::set_parent_visible(bool parent_visible) {
    if (_parent_visible && parent_visible) {
        return;
    }
    _parent_visible = parent_visible;
    _set_visible(_visible && _parent_visible);
}

void VoxelBlock::set_needs_lodding(bool need_lodding) {
    _needs_lodding = need_lodding;
}

bool VoxelBlock::is_modified() const {
    return _modified;
}

void VoxelBlock::set_modified(bool modified) {
    //	if (_modified != modified) {
    //		print_line(String("Marking block {0}[lod{1}] as modified").format(varray(bpos.to_vec3(), lod_index)));
    //	}
    _modified = modified;
}
