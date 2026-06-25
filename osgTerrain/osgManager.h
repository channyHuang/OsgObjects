#pragma once

#include <mutex>
#include <unordered_map>

#include "commonMath/vector3i.h"
#include "generator/voxel_mesher.h"
#include "terrains/voxelBrush.h"
#include "generator/terrainModification.h"
#include "physics/physicsManager.h"
//#include "reconstruct.h"


#include "commonOsg/osgManagerBase.h"
#include "commonOsg/commonOsg.h"

class OsgManager : public OsgManagerBase
{
public:
	static OsgManager* getInstance() {
		static OsgManager* m_pInstance;
		if (m_pInstance == nullptr) {
			m_pInstance = new OsgManager();
		}
		return m_pInstance;
	}
	virtual ~OsgManager();

	std::string createSpheres(const osg::Vec3& pos);
#ifdef PHYSICS_ON
	void updateDynamicActors(const std::string& id, const physx::PxVec3& pxpos);
#endif
	void removeDynamicActors(const std::string& id);
	void showModelScene(const char* pName);
	bool Collaborate(const char* pName);
	void clear();
	void setNormalMap(osg::ref_ptr<osg::Node> geomNode, osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene);
	
// slots
	void updateTerrain(const Arrays& surface, Vector3i pos);

public:
	bool bReload = true;
	TerrainBrush terrainBrush;
	int modifyType;
	std::shared_ptr<VoxelBrush> m_pBrush;
#ifdef PHYSICS_ON
	std::queue<std::pair<std::string, physx::PxRigidDynamic*>> quDynamicActors;
#endif
	bool valid = false;
	float progress = 0.f;
	
private:
	OsgManager();
	void init();
	void updateShow();

	osg::ref_ptr<osg::Geometry> getMeshGeometry(const Arrays& surface, osg::Vec3 color = osg::Vec3(1.f, 1.f, 1.f));
	osg::ref_ptr<osgShadow::ShadowedScene> getShadowScene(osg::ref_ptr<osg::Geometry> geomNode, ShowType type = ShowType::SHOW_FRONT_AND_BACK);

	void showModelWithShader(osg::ref_ptr<osg::Geometry> pGeom);

protected:
	osg::ref_ptr<osg::Group> m_pRootGeomTerrain = nullptr;
	osg::ref_ptr<osg::Group> m_pRootWireTerrain = nullptr;
	osg::ref_ptr<osg::Group> sunLight = nullptr;
	osg::ref_ptr<osgViewer::Viewer> pViewer = nullptr;
	osg::BoundingSphere bs;

	bool bFirstTime = false;
	int sceneIdx = 0;
	int sceneMaxIdx = 0;

	//Reconstruct* precon = nullptr;

	std::unordered_map<Vector3i, std::pair<osg::ref_ptr<osg::Geometry>, osg::ref_ptr<osg::Geometry>>, Vector3iHash> m_mapScene;

	osg::Vec3 up, eye, center;
	float theta = 0, phi = 0;

	std::mutex mutex;
	// physics
	osg::ref_ptr<osg::Group> physicsRoot = nullptr;
	std::unordered_map<std::string, std::pair<osg::ref_ptr<osg::Geometry>, osg::ref_ptr<osg::MatrixTransform>>> physicsSphere;
	std::queue<std::string> qu;
	// test
	std::ifstream ifs;
	
	// collaborate
	osg::ref_ptr<osg::Geometry> geom1, geom2;
	osg::ref_ptr<osg::MatrixTransform> transform1, transform2;
	bool bInitCollaborate = false;
	osg::ref_ptr<osg::Group> pCollaborateRoot = nullptr;
	std::vector<std::vector<float>> matTerrain;
	float halfDim = 50.f;
	// recon frame
	int curIdx = 0;
};

