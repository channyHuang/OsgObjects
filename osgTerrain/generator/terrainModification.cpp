#include "terrainModification.h"

TerrainModification* TerrainModification::instance = nullptr;

void TerrainModification::updateBrush(const TerrainBrush& brush) {
	terrainBrush = brush;
}

void TerrainModification::modify(TerrainModifyType modifyType, const Vector3& center) {
	if (!activate) return;
	switch (modifyType) {
	case Modify_Add:
		add(center);
		break;
	default:
		break;
	}
}

void TerrainModification::add(const Vector3& center) {
	std::cout << __FUNCTION__ << " " << center.toString() << std::endl;
	Vector3 vmin = center, vmax = center;
	switch (terrainBrush.eBrushType) {
	case BrushSphere:
		vmin = center - terrainBrush.radius;
		vmax = center + terrainBrush.radius;
		break;
	case BrushSquare:
		vmin = center - terrainBrush.size;
		vmax = center + terrainBrush.size;
		break;
	default:
		break;
	}

	Vector3i&& vmin_pos = MathFuncs::vector3FloorOrCeil(vmin, true);
	Vector3i&& vmax_pos = MathFuncs::vector3FloorOrCeil(vmax, false);

	MaterialType ematerial_origin = MaterialType::AIR;
	Vector3 pos = Vector3(0), selectionSize = vmax - vmin;
	Vector3 vhalf_selection_size = selectionSize * .5f;
	Box box(vmin, vmax);

	for (pos.x = vmin_pos.x; pos.x <= vmax_pos.x; ++pos.x) {
		for (pos.z = vmin_pos.z; pos.z <= vmax_pos.z; ++pos.z) {
			for (pos.y = vmin_pos.y; pos.y <= vmax_pos.y; ++pos.y) {
				float sdf = calcSdf(center, box, pos);


			}
		}
	}
}

float TerrainModification::calcSdf(const Vector3& center, const Box& box, const Vector3& pos) {
	float sdf = 1.f;
	switch (terrainBrush.eBrushType) {
	case BrushSphere:
		sdf = (center - pos).len() - (box.vMax.y - box.vMin.y) * 0.5f;
		break;
	case BrushSquare:
		sdf = MathFuncs::minDistToCube(pos, box);
		break;
	}
	return sdf;
}