#ifndef VOXEL_MESHER_H
#define VOXEL_MESHER_H

#include "../voxels/voxelBuffer.h"
#include "commonMath/vector3.h"
#include "terrains/commonStruct.h"


class VoxelMesher {
public:
	

	virtual void build(MeshOutput &MeshOutput, const MeshInput &voxels);

	int get_minimum_padding() const;
	int get_maximum_padding() const;

	virtual VoxelMesher *clone();

protected:
	static void _bind_methods();

	void set_padding(int minimum, int maximum);

private:
    int _minimum_padding = 2;
    int _maximum_padding = 2;
};

#endif // VOXEL_MESHER_H
