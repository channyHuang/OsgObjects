#pragma once

#include "terrains/commonStruct.h"

class VoxelMesher {
public:
	int get_minimum_padding() const;
	int get_maximum_padding() const;
	void set_padding(int minimum, int maximum);

	virtual void build(MeshOutput &MeshOutput, const MeshInput &voxels);

private:
    int _minimum_padding = 2;
    int _maximum_padding = 2;
};
