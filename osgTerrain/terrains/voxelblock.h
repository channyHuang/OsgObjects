#ifndef VOXELBLOCK_H
#define VOXELBLOCK_H


#include "commonMath/vector3i.h"
#include "commonMath/vector3.h"
#include "voxels/voxelBuffer.h"

//#define VOXEL_DEBUG_LOD_MATERIALS

class Spatial;

// Internal structure holding a reference to mesh visuals, physics and a block of voxel data.
class VoxelBlock {
public:
    enum MeshState {
        MESH_NEVER_UPDATED = 0, // TODO Redundant with MESH_NEED_UPDATE?
        MESH_UP_TO_DATE,
        MESH_NEED_UPDATE, // The mesh is out of date but was not yet scheduled for update
        MESH_UPDATE_NOT_SENT, // The mesh is out of date and was scheduled for update, but no request have been sent yet
        MESH_UPDATE_SENT // The mesh is out of date, and an update request was sent, pending response
    };

    VoxelBuffer* voxels;
    Vector3i position;
    unsigned int lod_index = 0;
    bool pending_transition_update = false;

    static VoxelBlock *create(Vector3i bpos, VoxelBuffer* buffer, unsigned int size, unsigned int p_lod_index);

    ~VoxelBlock();

    // Visuals and physics

    void set_mesh_state(MeshState ms);
    MeshState get_mesh_state() const;

    void set_visible(bool visible);
    bool is_visible() const;

    void set_parent_visible(bool parent_visible);

    void set_transition_mask(uint8_t m);
    //void set_transition_bit(uint8_t side, bool value);
    inline uint8_t get_transition_mask() const { return _transition_mask; }

    // Voxel data

    void set_needs_lodding(bool need_lodding);
    inline bool get_needs_lodding() const { return _needs_lodding; }

    bool is_modified() const;
    void set_modified(bool modified);

private:
    VoxelBlock();

    void _set_visible(bool visible);
    inline bool _is_transition_visible(int side) const { return _transition_mask & (1 << side); }


private:
    Vector3i _position_in_voxels;

#ifdef VOXEL_DEBUG_LOD_MATERIALS
    Ref<Material> _debug_material;
    Ref<Material> _debug_transition_material;
#endif

    int _mesh_update_count = 0;
    bool _visible = true;
    bool _parent_visible = true;
    MeshState _mesh_state = MESH_NEVER_UPDATED;
    uint8_t _transition_mask = 0;

    // The block was edited, which requires its LOD counterparts to be recomputed
    bool _needs_lodding = false;

    // Indicates if this block is different from the time it was loaded (should be saved)
    bool _modified = false;
};


#endif // VOXELBLOCK_H
