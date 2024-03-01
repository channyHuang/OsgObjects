#ifndef TERRAINMODIFICATION_H
#define TERRAINMODIFICATION_H

#include "voxels/common_enum.h"
#include "commonMath/vector3i.h"
#include "commonMath/vector3.h"
#include "commonMath/funcs.h"

class TerrainBrush {
public:
	TerrainBrush() : eBrushType(BrushSphere) {}
	~TerrainBrush() {}

	TerrainBrush& operator = (const TerrainBrush& brush) {
		eBrushType = brush.eBrushType;
		center = brush.center;
		switch (eBrushType) {
		case BrushSphere:
			radius = brush.radius;
			break;
		case BrushSquare:
			size = brush.size;
			break;
		default:
			break;
		}
		return *this;
	}

public:
	int eBrushType;
	float radius = 2.f;
	Vector3 size = Vector3(2);
	Vector3 center;
	int material = GRASSLAND;
};

class TerrainModification {
public:
	static TerrainModification* getInstance() {
		if (instance == nullptr) {
			instance = new TerrainModification;
		}
		return instance;
	}

	void updateBrush(const TerrainBrush& brush);
	void modify(TerrainModifyType modifyType, const Vector3& center);

public:
	bool activate = false;

private:
	TerrainModification() {}

	void add(const Vector3& center);
	float calcSdf(const Vector3& center, const Box& box, const Vector3& pos);

private:
	TerrainBrush terrainBrush;

	static TerrainModification* instance;
};

#endif