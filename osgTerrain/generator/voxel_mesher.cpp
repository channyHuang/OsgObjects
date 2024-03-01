#include "voxel_mesher.h"

void VoxelMesher::build(MeshOutput &output, const MeshInput &input) {
}

int VoxelMesher::get_minimum_padding() const {
	return _minimum_padding;
}

int VoxelMesher::get_maximum_padding() const {
	return _maximum_padding;
}

void VoxelMesher::set_padding(int minimum, int maximum) {
	_minimum_padding = minimum;
	_maximum_padding = maximum;
}

VoxelMesher *VoxelMesher::clone() {
	return nullptr;
}
