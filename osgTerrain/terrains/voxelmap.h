#ifndef VOXELMAP_H
#define VOXELMAP_H

#include <unordered_map>

#include "commonMath/boxi.h"
#include "voxelblock.h"

typedef Boxi Rect3i;

// Infinite voxel storage by means of octants like Gridmap, within a constant LOD
class VoxelMap {
public:
    // Converts voxel coodinates into block coordinates.
    // Don't use division because it introduces an offset in negative coordinates.
    static inline Vector3i voxel_to_block_b(Vector3i pos, int block_size_pow2) {
        return pos >> block_size_pow2;
    }

    inline Vector3i voxel_to_block(Vector3i pos) const {
        return voxel_to_block_b(pos, _block_size_pow2);
    }

    inline Vector3i to_local(Vector3i pos) const {
        return Vector3i(
                pos.x & _block_size_mask,
                pos.y & _block_size_mask,
                pos.z & _block_size_mask);
    }

    // Converts block coodinates into voxel coordinates
    inline Vector3i block_to_voxel(Vector3i bpos) const {
        return bpos * _block_size;
    }

    VoxelMap();
    ~VoxelMap();

    static VoxelMap *getInstance() {
        if (instance == nullptr) {
            instance = new VoxelMap;
        }
        return instance;
    }

    void create(unsigned int block_size_po2, int lod_index);

    inline unsigned int get_block_size() const { return _block_size; }
    inline unsigned int get_block_size_pow2() const { return _block_size_pow2; }
    inline unsigned int get_block_size_mask() const { return _block_size_mask; }

    void set_lod_index(int lod_index);
    unsigned int get_lod_index() const;

    int get_voxel(Vector3i pos, unsigned int c = 0) const;
    void set_voxel(int value, Vector3i pos, unsigned int c = 0);

    float get_voxel_f(Vector3i pos, unsigned int c = VoxelBuffer::CHANNEL_SDF) const;
    void set_voxel_f(float value, Vector3i pos, unsigned int c = VoxelBuffer::CHANNEL_SDF);

    void set_default_voxel(int value, unsigned int channel = 0);
    int get_default_voxel(unsigned int channel = 0);

    // Gets a copy of all voxels in the area starting at min_pos having the same size as dst_buffer.
    void get_buffer_copy(Vector3i min_pos, std::shared_ptr<VoxelBuffer> dst_buffer, unsigned int channels_mask = 1);

    // Moves the given buffer into a block of the map. The buffer is referenced, no copy is made.
    VoxelBlock *set_block_buffer(Vector3i bpos, VoxelBuffer* buffer);

    struct NoAction {
        inline void operator()(VoxelBlock *block) {}
    };

    template <typename Action_T>
    void remove_block(Vector3i bpos, Action_T pre_delete) {
        if (_last_accessed_block && _last_accessed_block->position == bpos) {
            _last_accessed_block = nullptr;
        }
        VoxelBlock *pptr = _blocks[bpos];
        if (pptr) {
            VoxelBlock *block = pptr;
            pre_delete(block);
            delete(block);
            remove_block_internal(bpos);
        }
    }

    void remove_block(const Vector3i& bpos) {
        VoxelBlock* pptr = _blocks[bpos];
        if (pptr) {
            VoxelBlock* block = pptr;
            delete(block);
            remove_block_internal(bpos);
        }
    }

    VoxelBlock *get_block(Vector3i bpos) const;

    bool has_block(Vector3i pos) const;
    bool is_block_surrounded(Vector3i pos) const;

    void clear();

    int get_block_count() const;

    template <typename Op_T>
    void for_all_blocks(Op_T op) {
        const Vector3i key;
        for (auto itr = _blocks.begin(); itr != _blocks.end(); itr++) {
            VoxelBlock *block = itr->second;
            if (block != nullptr) {
                op(block);
            }
        }
    }

    bool is_area_fully_loaded(const Rect3i voxels_box) const;

private:
    static VoxelMap *instance;

    void set_block(Vector3i bpos, VoxelBlock *block);
    VoxelBlock *get_or_create_block_at_voxel_pos(Vector3i pos);
    void remove_block_internal(Vector3i bpos);

    void set_block_size_pow2(unsigned int p);

    static void _bind_methods();

    int _b_get_voxel(int x, int y, int z, unsigned int c) { return get_voxel(Vector3i(x, y, z), c); }
    void _b_set_voxel(int value, int x, int y, int z, unsigned int c) { set_voxel(value, Vector3i(x, y, z), c); }
    float _b_get_voxel_f(int x, int y, int z, unsigned int c) { return get_voxel_f(Vector3i(x, y, z), c); }
    void _b_set_voxel_f(float value, int x, int y, int z, unsigned int c) { set_voxel_f(value, Vector3i(x, y, z), c); }
    int _b_get_voxel_v(Vector3 pos, unsigned int c) { return get_voxel(Vector3i(pos), c); }
    void _b_set_voxel_v(int value, Vector3 pos, unsigned int c) { set_voxel(value, Vector3i(pos), c); }
    bool _b_has_block(int x, int y, int z) { return has_block(Vector3i(x, y, z)); }
    Vector3 _b_voxel_to_block(Vector3 pos) const { return voxel_to_block(Vector3i(pos)).to_vec3(); }
    Vector3 _b_block_to_voxel(Vector3 pos) const { return block_to_voxel(Vector3i(pos)).to_vec3(); }
    bool _b_is_block_surrounded(Vector3 pos) const { return is_block_surrounded(Vector3i(pos)); }
    void _b_get_buffer_copy(Vector3 pos, VoxelBuffer* dst_buffer_ref, unsigned int channel = 0);
    void _b_set_block_buffer(Vector3 bpos, VoxelBuffer* buffer) { set_block_buffer(Vector3i(bpos), buffer); }

private:
    // Voxel values that will be returned if access is out of map bounds
    FixedArray<int, VoxelBuffer::MAX_CHANNELS> _default_voxel;

    // TODO Consider using OAHashMap
    // Blocks stored with a spatial hash in all 3D directions
    std::unordered_map<Vector3i, VoxelBlock *, Vector3iHasher> _blocks;

    // Voxel access will most frequently be in contiguous areas, so the same blocks are accessed.
    // To prevent too much hashing, this reference is checked before.
    mutable VoxelBlock *_last_accessed_block;

    unsigned int _block_size;
    unsigned int _block_size_pow2;
    unsigned int _block_size_mask;

    unsigned int _lod_index = 0;
};


#endif // VOXELMAP_H
