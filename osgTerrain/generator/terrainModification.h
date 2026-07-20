#pragma once

#include "voxels/common_enum.h"
#include "commonMath/vector3i.h"
#include "commonMath/vector3.h"
#include "commonMath/funcs.h"
#include "commonGeometry/geometrymath.h"

#include <memory>

class TerrainBrush {
public:
	TerrainBrush() {}
	~TerrainBrush() {}

	TerrainBrush& operator = (const TerrainBrush& brush) {
		m_eBrushType = brush.m_eBrushType;
		m_vCenter = brush.m_vCenter;
		m_fSize = brush.m_fSize;
		return *this;
	}

public:
	int m_eBrushType = TerrainBrushType::BrushSphere;
	uint32_t m_eMetarial = MaterialType::GRASSLAND;
	float m_fSize = 10.f;
	Vector3 m_vCenter = Vector3(0, 0, 0);
};

class TerrainModification {
public:
	static TerrainModification* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new TerrainModification;
		}
		return m_pInstance;
	}

	void modify(TerrainModifyType eModifyType, const Vector3& vCenter);

public:
	bool m_bActivate = false;
	std::shared_ptr<TerrainBrush> m_pTerrainBrush;
	int m_eModifyType;

private:
	TerrainModification();
	~TerrainModification();

	void add(const Vector3& vCenter);
	void reduce(const Vector3& vCenter);
	float calcSdf(const Vector3& center, const Vector3& pos);

private:
	static TerrainModification* m_pInstance;

};
