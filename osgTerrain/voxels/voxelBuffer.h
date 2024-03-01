#pragma once

#include "commonMath/vector3i.h"
#include "commonMath/boxi.h"

#include "voxel_constants.h"
#include "array_slice.h"
#include "fixed_array.h"

union IntOrFloat {
    int i = 0;
    float f;

    inline bool operator == (const IntOrFloat &v) const {
        return (v.i == i && v.f == f);
    }

    inline bool operator != (const IntOrFloat &v) const {
        return (v.i != i && v.f != f);
    }
};

class VoxelBuffer {
public:
	enum ChannelId {
		CHANNEL_TYPE = 0,
		CHANNEL_SDF,
		CHANNEL_COLOR,
		MAX_CHANNELS
	};

	VoxelBuffer();
	~VoxelBuffer();

	void create(unsigned int sx, unsigned int sy, unsigned int sz);
	void create(Vector3i size);
	void clear();
    void clear_channel(unsigned int channel_index, int clear_value = 0);
    void clear_channel_f(unsigned int channel_index, float clear_value);

    void set_default_values(FixedArray<int, VoxelBuffer::MAX_CHANNELS> values);

    void set_voxel(int value, int x, int y, int z, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF);
    void set_voxel_f(float value, int x, int y, int z, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF);
    void set_voxel_raw(IntOrFloat value, int x, int y, int z, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF);
    int get_voxel(int x, int y, int z, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF) const;
    float get_voxel_f(int x, int y, int z, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF) const;
    IntOrFloat get_voxel_raw(int x, int y, int z, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF) const;

    void fill(int defval, unsigned int channel_index = 0);
    void fill(IntOrFloat defval, unsigned int channel_index = 0);
    void fill_f(float defval, unsigned int channel_index = 0);
    void fill_area(int defval, Vector3i min, Vector3i max, unsigned int channel_index = 0);
    void fill_area(IntOrFloat defval, Vector3i min, Vector3i max, unsigned int channel_index = 0);
    void fill_area_f(float defval, Vector3i min, Vector3i max, unsigned int channel_index);

	void copy_from(const VoxelBuffer &other);
	void copy_from(const VoxelBuffer &other, unsigned int channel_index);
    void copy_from(const VoxelBuffer &other, Vector3i& src_min, Vector3i& src_max, Vector3i& dst_min,
			unsigned int channel_index);

    inline const Vector3i &get_size() const { return _size; }

    static uint32_t get_size_in_bytes_for_volume(const Vector3i& size);
    void downscale_to(VoxelBuffer &dst,  Vector3i& src_min,  Vector3i& src_max,  Vector3i& dst_min) const;

    float get_voxel_f(const Vector3i &pos, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF) const {
        return get_voxel_f(pos.x, pos.y, pos.z, channel_index);
    }
    float get_voxel_f(const Vector3 &pos, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF) const {
        return get_voxel_f(pos.x, pos.y, pos.z, channel_index);
    }
    inline int get_voxel(const Vector3i pos, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF) const {
        return get_voxel(pos.x, pos.y, pos.z, channel_index);
    }
    inline void set_voxel(int value, const Vector3i pos, unsigned int channel_index = VoxelBuffer::CHANNEL_SDF) {
        set_voxel(value, pos.x, pos.y, pos.z, channel_index);
    }

    inline bool is_position_valid(unsigned int x, unsigned int y, unsigned int z) const {
		return x < (unsigned)_size.x && y < (unsigned)_size.y && z < (unsigned)_size.z;
	}

    inline bool is_position_valid(const Vector3i& pos) const {
		return is_position_valid(pos.x, pos.y, pos.z);
	}

    inline bool is_box_valid(const Boxi& box) const {
        return Boxi(Vector3i(), _size).contains(box);
	}

    static inline unsigned int get_index(const Vector3i& pos, const Vector3i& size) {
		return pos.get_zxy_index(size);
	}

    inline unsigned int get_index(unsigned int x, unsigned int y, unsigned int z) const {
		return y + _size.y * (x + _size.x * z); // ZXY index
	}

    inline unsigned int get_volume() const {
		return _size.x * _size.y * _size.z;
	}

private:
    void create_channel_noinit(int i, const Vector3i& size);
    void create_channel(int i, const Vector3i& size, IntOrFloat defval);
	void delete_channel(int i);

private:
	struct Channel {
		// Flat array, in order [z][x][y] because it allows faster vertical-wise access (the engine is Y-up).
        IntOrFloat *data = nullptr;
		// Default value when data is null
        IntOrFloat defval;
        bool bIsInt = true;
        int size_in_bytes = 0;
	};

    FixedArray<Channel, MAX_CHANNELS> _channels;
	Vector3i _size = Vector3i(0);
};
