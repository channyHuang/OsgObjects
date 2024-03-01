#ifndef UTILITY_H
#define UTILITY_H

#include "commonMath/vector3.h"
#include <vector>

struct SubMeshArrays {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<uint32_t> indices;
    bool isWater = false;

    bool empty() const
    {
        return positions.empty();
    }
};

inline bool is_surface_triangulated(SubMeshArrays subMeshArrays) {
    return subMeshArrays.positions.size() >= 3 && subMeshArrays.indices.size() >= 3;
}

// Takes elements starting from a given position and moves them at the beginning,
// then shrink the array to fit them. Other elements are discarded.

template <typename T>
void shift_up(std::vector<T> &v, unsigned int pos) {
    unsigned int j = 0;
    for (unsigned int i = pos; i < v.size(); ++i, ++j) {
        v[j] = v[i];
    }
    int remaining = v.size() - pos;
    v.resize(remaining);
}

// Removes all items satisfying the given predicate.
// This can change the size of the container, and original order of items is not preserved.
template <typename T, typename F>
inline void unordered_remove_if(std::vector<T> &vec, F predicate) {
    for (unsigned int i = 0; i < vec.size(); ++i) {
        if (predicate(vec[i])) {
            vec[i] = vec.back();
            vec.pop_back();
            // Note: can underflow, but it should be fine since it's incremented right after.
            // TODO Use a while()?
            --i;
        }
    }
}

// Removes all items satisfying the given predicate.
// This can reduce the size of the container. Items are moved to preserve order.
//template <typename T, typename F>
//inline void remove_if(std::vector<T> &vec, F predicate) {
//	unsigned int j = 0;
//	for (unsigned int i = 0; i < vec.size(); ++i) {
//		if (predicate(vec[i])) {
//			continue;
//		} else {
//			if (i != j) {
//				vec[j] = vec[i];
//			}
//			++j;
//		}
//	}
//	vec.resize(j);
//}

// template <typename T>
// void copy_to(PoolVector<T> &to, const Vector<T> &from) {
// 	to.resize(from.size());
// 	typename PoolVector<T>::Write w = to.write();
// 	for (unsigned int i = 0; i < from.size(); ++i) {
// 		w[i] = from[i];
// 	}
// }

template <typename T>
void copy_to(std::vector<T> &to, const std::vector<T> &from) {
    size_t right_n = from.size();
    if (right_n < 1) return;

    size_t left_n = to.size();
    to.resize(left_n + right_n);
    for (size_t i = 0; i < right_n; ++i)
    {
        to[left_n + i] = from[i];
    }
}

// template <typename T>
// void raw_copy_to(PoolVector<T> &to, const std::vector<T> &from) {
// 	to.resize(from.size());
// 	typename PoolVector<T>::Write w = to.write();
// 	memcpy(w.ptr(), from.data(), from.size() * sizeof(T));
// }

template <typename T>
void raw_copy_to(std::vector<T> &to, const std::vector<T> &from) {
    //todo
}

// TODO Move math funcs under math/ folder and wrap them in a namespace

// Trilinear interpolation between corner values of a cube.
// Cube points respect the same position as in octree_tables.h
template <typename T>
inline T interpolate(const T v0, const T v1, const T v2, const T v3, const T v4, const T v5, const T v6, const T v7, Vector3 position) {
    const float one_min_x = 1.f - position.x;
    const float one_min_y = 1.f - position.y;
    const float one_min_z = 1.f - position.z;
    const float one_min_x_one_min_y = one_min_x * one_min_y;
    const float x_one_min_y = position.x * one_min_y;

    T res = one_min_z * (v0 * one_min_x_one_min_y + v1 * x_one_min_y + v4 * one_min_x * position.y);
    res += position.z * (v3 * one_min_x_one_min_y + v2 * x_one_min_y + v7 * one_min_x * position.y);
    res += position.x * position.y * (v5 * one_min_z + v6 * position.z);

    return res;
}

template <typename T>
inline T clamp(const T x, const T min_value, const T max_value) {
    if (x < min_value) {
        return min_value;
    }
    if (x >= max_value) {
        return max_value;
    }
    return x;
}

template <typename T>
inline T squared(const T x) {
    return x * x;
}
/*
template <typename T>
inline void sort_min_max(T &a, T &b) {
    if (a > b) {
        T temp = a;
        a = b;
        b = temp;
    }
}*/
// bool is_surface_triangulated(Array surface);
// bool is_mesh_empty(Ref<Mesh> mesh_ref);

template <typename T>
inline void append_array(std::vector<T> &dst, const std::vector<T> &src) {
    dst.insert(dst.end(), src.begin(), src.end());
}

inline void appendSubMeshArrays(SubMeshArrays &dst, const SubMeshArrays &src) {
    const size_t indexBegin = dst.positions.size();
    append_array(dst.positions, src.positions);
    append_array(dst.normals, src.normals);
    auto temp = src.indices;
    for (size_t i = 0; i < temp.size(); i++)
    {
        temp[i] += indexBegin;
    }
    append_array(dst.indices, temp);
}

inline int udiv(int x, int d) {
    if (x < 0) {
        return (x - d + 1) / d;
    } else {
        return x / d;
    }
}

// TODO Rename `wrapi`
// `MathFuncs::wrapi` with zero min
inline int wrap(int x, int d) {
    return ((unsigned int)x - (x < 0)) % (unsigned int)d;
    //return ((x % d) + d) % d;
}
/*
// MathFuncs::wrapf with zero min
inline float wrapf(float x, float d) {
    return MathFuncs::IsNearlyZero(d) ? 0.f : x - (d * MathFuncs::Floor(x / d));
}

// Similar to MathFuncs::smoothstep but doesn't use macro to clamp
inline float smoothstep(float p_from, float p_to, float p_weight) {
    if (MathFuncs::IsEqual(p_from, p_to)) {
        return p_from;
    }
    float x = clamp((p_weight - p_from) / (p_to - p_from), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

inline unsigned long get_ticks_usec()
{
    return Time::Instance()->getMicroseconds();
}
//返回引擎启动后经过的时间(以毫秒为单位)。
inline uint32_t get_ticks_msec()
{
    return Time::Instance()->getMilliseconds();
}
*/
inline bool is_mesh_empty(const std::vector<SubMeshArrays>& array)
{
    if (array.empty())
    {
        return true;
    }
    for (size_t i = 0; i < array.size(); ++i)
    {
        if (!array[i].positions.empty())
        {
            return false;
        }
    }
    return true;
}

#if TOOLS_ENABLED
namespace VoxelDebug {
void create_debug_box_mesh();
void free_debug_box_mesh();
Ref<Mesh> get_debug_box_mesh();
} // namespace VoxelDebug
#endif


#endif // UTILITY_H
