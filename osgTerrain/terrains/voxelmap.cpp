#include "voxelmap.h"

#include "voxelblock.h"
#include <fstream>

VoxelMap *VoxelMap::instance = nullptr;

VoxelMap::VoxelMap() :
        _last_accessed_block(nullptr) {

    // TODO Make it configurable in editor (with all necessary notifications and updatings!)
    set_block_size_pow2(4);

    _default_voxel.fill(0);
    //_default_voxel[VoxelBuffer::CHANNEL_SDF] = 1;
}

VoxelMap::~VoxelMap() {
    clear();
}

void VoxelMap::create(unsigned int block_size_po2, int lod_index) {
    clear();
    set_block_size_pow2(block_size_po2);
    set_lod_index(lod_index);
}

void VoxelMap::set_block_size_pow2(unsigned int p) {
    _block_size_pow2 = p;
    _block_size = 1 << _block_size_pow2;
    _block_size_mask = _block_size - 1;
}

void VoxelMap::set_lod_index(int lod_index) {
    _lod_index = lod_index;
}

unsigned int VoxelMap::get_lod_index() const {
    return _lod_index;
}

int VoxelMap::get_voxel(Vector3i pos, unsigned int c) const {
    Vector3i bpos = voxel_to_block(pos);
    const VoxelBlock *block = get_block(bpos);
    if (block == nullptr) {
        return _default_voxel[c];
    }
    return block->voxels->get_voxel(to_local(pos), c);
}

VoxelBlock *VoxelMap::get_or_create_block_at_voxel_pos(Vector3i pos) {
    Vector3i bpos = voxel_to_block(pos);
    VoxelBlock *block = get_block(bpos);

    if (block == nullptr) {

        VoxelBuffer* buffer(new(VoxelBuffer));
        buffer->create(_block_size, _block_size, _block_size);
        buffer->set_default_values(_default_voxel);

        block = VoxelBlock::create(bpos, buffer, _block_size, _lod_index);

        set_block(bpos, block);
    }

    return block;
}

void VoxelMap::set_voxel(int value, Vector3i pos, unsigned int c) {
    VoxelBlock *block = get_or_create_block_at_voxel_pos(pos);
    block->voxels->set_voxel(value, to_local(pos), c);
}

float VoxelMap::get_voxel_f(Vector3i pos, unsigned int c) const {
    Vector3i bpos = voxel_to_block(pos);
    const VoxelBlock *block = get_block(bpos);
    if (block == nullptr) {
        return 1.f;
    }
    Vector3i lpos = to_local(pos);
    return block->voxels->get_voxel_f(lpos.x, lpos.y, lpos.z, c);
}

void VoxelMap::set_voxel_f(float value, Vector3i pos, unsigned int c) {
    VoxelBlock *block = get_or_create_block_at_voxel_pos(pos);
    Vector3i lpos = to_local(pos);
    block->voxels->set_voxel_f(value, lpos.x, lpos.y, lpos.z, c);
}

void VoxelMap::set_default_voxel(int value, unsigned int channel) {
    _default_voxel[channel] = value;
}

int VoxelMap::get_default_voxel(unsigned int channel) {
    return _default_voxel[channel];
}

VoxelBlock *VoxelMap::get_block(Vector3i bpos) const {
    if (_last_accessed_block && _last_accessed_block->position == bpos) {
        return _last_accessed_block;
    }
    VoxelBlock *p = nullptr;
    auto itr = _blocks.find(bpos);
    if (itr != _blocks.end()) {
        p = itr->second;
    }
    if (p) {
        _last_accessed_block = p;
        return _last_accessed_block;
    }
    return nullptr;
}

void VoxelMap::set_block(Vector3i bpos, VoxelBlock *block) {
    if (_last_accessed_block == nullptr || _last_accessed_block->position == bpos) {
        _last_accessed_block = block;
    }
    _blocks[bpos] = block;
}

void VoxelMap::remove_block_internal(Vector3i bpos) {
    // This function assumes the block is already freed
    _blocks.erase(bpos);
}

VoxelBlock *VoxelMap::set_block_buffer(Vector3i bpos, VoxelBuffer* buffer) {
    VoxelBlock *block = get_block(bpos);
    if (block == nullptr) {
        block = VoxelBlock::create(bpos, buffer, _block_size, _lod_index);
        set_block(bpos, block);
    } else {
        block->voxels = buffer;
    }
    return block;
}

bool VoxelMap::has_block(Vector3i pos) const {
    return /*(_last_accessed_block != nullptr && _last_accessed_block->pos == pos) ||*/ _blocks.find(pos) != _blocks.end();
}

bool VoxelMap::is_block_surrounded(Vector3i pos) const {

    return true;
}

void VoxelMap::get_buffer_copy(Vector3i min_pos, std::shared_ptr<VoxelBuffer> dst_buffer, unsigned int channels_mask) {
    Vector3i max_pos = min_pos + dst_buffer->get_size();

    Vector3i min_block_pos = voxel_to_block(min_pos);
    Vector3i max_block_pos = voxel_to_block(max_pos - Vector3i(1, 1, 1)) + Vector3i(1, 1, 1);
    // TODO Why is this function limited by this check?

    const Vector3i block_size_v(_block_size, _block_size, _block_size);
#if 1
    for (unsigned int channel = 0; channel < VoxelBuffer::MAX_CHANNELS; ++channel) {
        if (((1 << channel) & channels_mask) == 0) {
            continue;
        }

        Vector3i bpos;
        for (bpos.z = min_block_pos.z; bpos.z < max_block_pos.z; ++bpos.z) {
            for (bpos.x = min_block_pos.x; bpos.x < max_block_pos.x; ++bpos.x) {
                for (bpos.y = min_block_pos.y; bpos.y < max_block_pos.y; ++bpos.y) {
                    VoxelBlock *block = get_block(bpos);

                    if (block) {
                        VoxelBuffer &src_buffer = *block->voxels;

                        Vector3i offset = block_to_voxel(bpos);
                        Vector3i src_min = min_pos - offset;
                        Vector3i src_max = src_buffer.get_size();
                        Vector3i dst_min = Vector3i(0);
                        // Note: copy_from takes care of clamping the area if it's on an edge
                        dst_buffer->copy_from(src_buffer,
                                src_min,
                                src_max,
                                dst_min,
                                channel);

                    } else {
                        // For now, inexistent blocks default to hardcoded defaults, corresponding to "empty space".
                        // If we want to change this, we may have to add an API for that it in `VoxelStream`.
                        Vector3i offset = block_to_voxel(bpos);
                        if (channel == VoxelBuffer::CHANNEL_SDF) {
                            dst_buffer->fill_area_f(
                                1.f,
                                offset - min_pos,
                                offset - min_pos + block_size_v,
                                channel);
                        }
                        else {
                            dst_buffer->fill_area(
                                _default_voxel[channel],
                                offset - min_pos,
                                offset - min_pos + block_size_v,
                                channel);
                        }
                    }
                }
            }
        }
    }
#endif
}

void VoxelMap::clear() {
    const Vector3i *key = nullptr;
    /*while ((key = _blocks.next(key))) {
        VoxelBlock *block_ptr = _blocks.get(*key);
        if (block_ptr == nullptr) {
        }
        delete(block_ptr);
    }*/
    _blocks.clear();
    _last_accessed_block = nullptr;
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

int VoxelMap::get_block_count() const {
    return _blocks.size();
}

bool VoxelMap::is_area_fully_loaded(const Rect3i voxels_box) const {
    //Rect3i block_box = voxels_box.downscaled(get_block_size());
    //return block_box.all_cells_match([this](Vector3i pos) {
    //    return has_block(pos);
    //});
    return false;
}
