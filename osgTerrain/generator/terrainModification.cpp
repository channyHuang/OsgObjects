#include "terrainModification.h"

TerrainModification* TerrainModification::m_pInstance = nullptr;

TerrainModification::TerrainModification() {
	m_pTerrainBrush = std::make_shared<TerrainBrush>();
}

TerrainModification::~TerrainModification() {}

void TerrainModification::modify(TerrainModifyType eModifyType, const Vector3& vCenter) {
	if (!m_bActivate) return;
	switch (eModifyType) {
	case Modify_Add:
		add(vCenter);
		break;
	case Modify_Reduce:
		reduce(vCenter);
		break;
	default:
		break;
	}
}

void TerrainModification::add(const Vector3& vCenter) {
	Vector3 vMin = vCenter, vMax = vCenter;
	vMin = vCenter - Vector3(m_pTerrainBrush->m_fSize);
	vMax = vCenter + Vector3(m_pTerrainBrush->m_fSize);
	
	Vector3i&& vMinPos = MathFuncs::vector3FloorOrCeil(vMin, true);
	Vector3i&& vMaxPos = MathFuncs::vector3FloorOrCeil(vMax, false);

	MaterialType eMaterial = MaterialType::AIR;
	Vector3 vPos = Vector3(0);

	for (vPos.x = vMinPos.x; vPos.x <= vMaxPos.x; ++vPos.x) {
		for (vPos.z = vMinPos.z; vPos.z <= vMaxPos.z; ++vPos.z) {
			for (vPos.y = vMinPos.y; vPos.y <= vMaxPos.y; ++vPos.y) {
				
			}
		}
	}
}

void TerrainModification::reduce(const Vector3& vCenter) {}

float TerrainModification::calcSdf(const Vector3& vCenter, const Vector3& vPos) {
	float sdf = 1.f;
	switch (m_pTerrainBrush->m_eBrushType) {
	case BrushSphere:
		sdf = (vCenter - vPos).len() - m_pTerrainBrush->m_fSize * 0.5f;
		break;
	case BrushSquare:
		// sdf = MathFuncs::minDistToCube(vPos, box);
		break;
	default:
		break;
	}
	return sdf;
}