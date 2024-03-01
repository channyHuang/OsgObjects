#include "voxelBuffer.h"

#include "commonMath/funcs.h"
#include <string.h>

inline IntOrFloat *allocate_channel_data(uint32_t size) {
    return (IntOrFloat *)malloc(size);
}

inline void free_channel_data(IntOrFloat *data, uint32_t size) {
    free(data);
}

VoxelBuffer::VoxelBuffer() {
    _channels[CHANNEL_TYPE].bIsInt = true;
    _channels[CHANNEL_TYPE].defval.i = 0;

    _channels[CHANNEL_SDF].bIsInt = false;
    _channels[CHANNEL_SDF].defval.f = 1.f;
}

VoxelBuffer::~VoxelBuffer() {
	clear();
}

void VoxelBuffer::create(unsigned int sx, unsigned int sy, unsigned int sz) {
	Vector3i new_size(sx, sy, sz);
	if (new_size != _size) {
        _size = new_size;
		for (unsigned int i = 0; i < MAX_CHANNELS; ++i) {
			Channel &channel = _channels[i];
            if (channel.data) {
				delete_channel(i);
			}
            create_channel(i, new_size, channel.defval);
		}
	}
}

void VoxelBuffer::create(Vector3i size) {
	create(size.x, size.y, size.z);
}

void VoxelBuffer::clear() {
	//for (unsigned int i = 0; i < MAX_CHANNELS; ++i) {
	//	Channel &channel = _channels[i];
	//	if (channel.data) {
	//		delete_channel(i);
	//	}
	//}
    _size = Vector3i();
}

void VoxelBuffer::clear_channel(unsigned int channel_index, int clear_value) {
	Channel &channel = _channels[channel_index];
	if (channel.data != nullptr) {
		delete_channel(channel_index);
	}
    channel.defval.i = clear_value;
}

void VoxelBuffer::clear_channel_f(unsigned int channel_index, float clear_value) {
	const Channel &channel = _channels[channel_index];
    IntOrFloat value;
    value.f = clear_value;
    clear_channel(channel_index, value.i);
}

void VoxelBuffer::set_default_values(FixedArray<int, VoxelBuffer::MAX_CHANNELS> values) {
	//for (unsigned int i = 0; i < MAX_CHANNELS; ++i) {
 //       IntOrFloat value;
 //       value.i = values[i];

 //       _channels[i].defval = value;
	//}
}

void VoxelBuffer::set_voxel(int value, int x, int y, int z, unsigned int channel_index) {
    IntOrFloat fvalue;
    fvalue.i = value;
    set_voxel_raw(fvalue, x, y, z, channel_index);
}

void VoxelBuffer::set_voxel_f(float value, int x, int y, int z, unsigned int channel_index) {
    IntOrFloat fvalue;
    fvalue.f = value;
    set_voxel_raw(fvalue, x, y, z, channel_index);
}

void VoxelBuffer::set_voxel_raw(IntOrFloat value, int x, int y, int z, unsigned int channel_index) {
    Channel &channel = _channels[channel_index];
    bool do_set = true;
    if (channel.data == nullptr) {
        if (channel.defval != value) {
            create_channel(channel_index, _size, channel.defval);
        } else {
            do_set = false;
        }
    }

    if (do_set) {
        const uint32_t i = get_index(x, y, z);
        channel.data[i] = value;
    }
}

int VoxelBuffer::get_voxel(int x, int y, int z, unsigned int channel_index) const {
    IntOrFloat value = get_voxel_raw(x, y, z, channel_index);
    return value.i;
}

float VoxelBuffer::get_voxel_f(int x, int y, int z, unsigned int channel_index) const {
    IntOrFloat value = get_voxel_raw(x, y, z, channel_index);
    return value.f;
}

IntOrFloat VoxelBuffer::get_voxel_raw(int x, int y, int z, unsigned int channel_index) const {
    const Channel &channel = _channels[channel_index];
    if (channel.data != nullptr) {
        const uint32_t i = get_index(x, y, z);
        return channel.data[i];
    }
    return channel.defval;
}

void VoxelBuffer::fill(int defval, unsigned int channel_index) {
    IntOrFloat value;
    value.i = defval;
    fill(value, channel_index);
}

void VoxelBuffer::fill(IntOrFloat defval, unsigned int channel_index) {
    Channel &channel = _channels[channel_index];

    if (channel.data == nullptr) {
        if (channel.defval == defval) {
			return;
        } else {
            channel.defval = defval;
			return;
		}
	}
    if (channel_index == CHANNEL_SDF) {
        for (int i = 0; i < get_volume(); ++i) {
            channel.data[i].f = 1.f;
        }
    }
    else {
        memset(channel.data, defval.i, channel.size_in_bytes);
    }
}

void VoxelBuffer::fill_f(float defval, unsigned int channel) {
    IntOrFloat value;
    value.f = defval;
    fill(value, channel);
}

void VoxelBuffer::fill_area(IntOrFloat defval, Vector3i min, Vector3i max, unsigned int channel_index) {
    Vector3i::sort_min_max(min, max);

    min.clamp_to(Vector3i(0, 0, 0), _size + Vector3i(1, 1, 1));
    max.clamp_to(Vector3i(0, 0, 0), _size + Vector3i(1, 1, 1));
    const Vector3i area_size = max - min;

    if (area_size.x == 0 || area_size.y == 0 || area_size.z == 0) {
        return;
    }

    Channel &channel = _channels[channel_index];
    if (channel.data == nullptr) {
        if (channel.defval == defval) {
            return;
        } else {
            create_channel(channel_index, _size, channel.defval);
        }
    }

    Vector3i pos;
    const unsigned int volume = get_volume();
    for (pos.z = min.z; pos.z < max.z; ++pos.z) {
        for (pos.x = min.x; pos.x < max.x; ++pos.x) {
            const unsigned int dst_ri = get_index(pos.x, pos.y + min.y, pos.z);

            if (channel_index == CHANNEL_SDF) {
                for (int i = 0; i < area_size.y; ++i) {
                    channel.data[dst_ri + i].f = defval.f;
                }
            }
            else {
                memset(&channel.data[dst_ri], defval.i, area_size.y * sizeof(IntOrFloat));
            }
        }
    }
}

void VoxelBuffer::fill_area(int defval, Vector3i min, Vector3i max, unsigned int channel_index) {
    IntOrFloat value;
    value.i = defval;
    fill_area(value, min, max, channel_index);
}

void VoxelBuffer::fill_area_f(float fvalue, Vector3i min, Vector3i max, unsigned int channel_index) {
    IntOrFloat value;
    value.f = fvalue;
    fill_area(value, min, max, channel_index);
}

void VoxelBuffer::copy_from(const VoxelBuffer &other) {
	for (unsigned int i = 0; i < MAX_CHANNELS; ++i) {
		copy_from(other, i);
	}
}

void VoxelBuffer::copy_from(const VoxelBuffer &other, unsigned int channel_index) {
    Channel &channel = _channels[channel_index];
	const Channel &other_channel = other._channels[channel_index];

	if (other_channel.data != nullptr) {
		if (channel.data == nullptr) {
			create_channel_noinit(channel_index, _size);
        }
		memcpy(channel.data, other_channel.data, channel.size_in_bytes);

	} else if (channel.data != nullptr) {
		delete_channel(channel_index);
	}

	channel.defval = other_channel.defval;
    channel.bIsInt = other_channel.bIsInt;
}

inline void clip_copy_region_coord(int &src_min, int &src_max, const int src_size, int &dst_min, const int dst_size) {
	// Clamp source and shrink destination for moved borders
	if (src_min < 0) {
		dst_min += -src_min;
		src_min = 0;
	}
	if (src_max > src_size) {
		src_max = src_size;
	}
	// Clamp destination and shrink source for moved borders
	if (dst_min < 0) {
		src_min += -dst_min;
		dst_min = 0;
	}
	const int dst_w = src_max - src_min;
	const int dst_max = dst_min + dst_w;
	if (dst_max > dst_size) {
		src_max -= dst_max - dst_size;
    }
}

inline void clip_copy_region(
        Vector3i &src_min, Vector3i &src_max, const Vector3i &src_size, Vector3i &dst_min, const Vector3i &dst_size) {
    clip_copy_region_coord(src_min.x, src_max.x, src_size.x, dst_min.x, dst_size.x);
    clip_copy_region_coord(src_min.y, src_max.y, src_size.y, dst_min.y, dst_size.y);
    clip_copy_region_coord(src_min.z, src_max.z, src_size.z, dst_min.z, dst_size.z);
}

void VoxelBuffer::copy_from(const VoxelBuffer &other, Vector3i &src_min, Vector3i &src_max, Vector3i &dst_min,
		unsigned int channel_index) {
	Channel &channel = _channels[channel_index];
	const Channel &other_channel = other._channels[channel_index];

    if (channel.data == nullptr && other_channel.data == nullptr && channel.defval == other_channel.defval) {
		return;
	}

	Vector3i::sort_min_max(src_min, src_max);

	clip_copy_region(src_min, src_max, other._size, dst_min, _size);

	const Vector3i area_size = src_max - src_min;

	if (area_size.x <= 0 || area_size.y <= 0 || area_size.z <= 0) {
		return;
	}

    if (area_size == _size && area_size == other._size) {
		copy_from(other, channel_index);

	} else {
		if (other_channel.data != nullptr) {
			if (channel.data == nullptr) {
				create_channel(channel_index, _size, channel.defval);
			}

            Vector3i pos;
            for (pos.z = 0; pos.z < area_size.z; ++pos.z) {
                for (pos.x = 0; pos.x < area_size.x; ++pos.x) {
                    const unsigned int src_ri =
                            other.get_index(pos.x + src_min.x, pos.y + src_min.y, pos.z + src_min.z);
                    const unsigned int dst_ri = get_index(pos.x + dst_min.x, pos.y + dst_min.y, pos.z + dst_min.z);
                    memcpy(&channel.data[dst_ri], &other_channel.data[src_ri], area_size.y * sizeof(IntOrFloat));
                }
            }
		} else if (channel.defval != other_channel.defval) {
			if (channel.data == nullptr) {
				create_channel(channel_index, _size, channel.defval);
			}
			fill_area(other_channel.defval, dst_min, dst_min + area_size, channel_index);
		}
	}
}

uint32_t VoxelBuffer::get_size_in_bytes_for_volume(const Vector3i& size) {
    const unsigned int volume = size.x * size.y * size.z;
    const unsigned int bits = volume * sizeof(float);
    const unsigned int size_in_bytes = bits/*(bits >> 3)*/;
    return size_in_bytes;
}

void VoxelBuffer::downscale_to(VoxelBuffer &dst,  Vector3i& src_min,  Vector3i& src_max,  Vector3i& dst_min) const {
    src_min.clamp_to(Vector3i(), _size);
    src_max.clamp_to(Vector3i(), _size + Vector3i(1));

    Vector3i dst_max = dst_min + ((src_max - src_min) >> 1);

    dst_min.clamp_to(Vector3i(), dst._size);
    dst_max.clamp_to(Vector3i(), dst._size + Vector3i(1));

    for (int channel_index = 0; channel_index < MAX_CHANNELS; ++channel_index) {
        const Channel &src_channel = _channels[channel_index];
        const Channel &dst_channel = dst._channels[channel_index];

        if (src_channel.data == nullptr && dst_channel.data == nullptr && src_channel.defval == dst_channel.defval) {
            continue;
        }

        // Nearest-neighbor downscaling

        Vector3i pos;
        for (pos.z = dst_min.z; pos.z < dst_max.z; ++pos.z) {
            for (pos.x = dst_min.x; pos.x < dst_max.x; ++pos.x) {
                for (pos.y = dst_min.y; pos.y < dst_max.y; ++pos.y) {
                    const Vector3i src_pos = src_min + ((pos - dst_min) << 1);
                    int v;
                    if (src_channel.data) {
                        v = get_voxel(src_pos, channel_index);
                    } else {
                        v = src_channel.defval.i;
                    }

                    dst.set_voxel(v, pos, channel_index);
                }
            }
        }
    }
}

// private

void VoxelBuffer::create_channel_noinit(int i, const Vector3i& size) {
	Channel &channel = _channels[i];
    uint32_t size_in_bytes = get_size_in_bytes_for_volume(size);
	channel.data = allocate_channel_data(size_in_bytes);
	channel.size_in_bytes = size_in_bytes;
}

void VoxelBuffer::create_channel(int i, const Vector3i& size, IntOrFloat defval) {
    create_channel_noinit(i, size);
    fill(defval, i);
}

void VoxelBuffer::delete_channel(int channel_index) {
    Channel &channel = _channels[channel_index];
	free_channel_data(channel.data, channel.size_in_bytes);
	channel.data = nullptr;
	channel.size_in_bytes = 0;
}
