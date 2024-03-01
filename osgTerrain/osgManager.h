#pragma once

#include <unordered_map>

#include "commonOsg.h"
#include "MeshPostProcessing.h"

#include "commonMath/vector3i.h"
#include "generator/voxel_mesher.h"
#include "terrains/voxelBrush.h"
#include "generator/terrainModification.h"
#include "physics/physicsManager.h"
//#include "reconstruct.h"

#include <mutex>

class OsgManager
{
public:
	static OsgManager* getInstance() {
		if (instance == nullptr) {
			instance = new OsgManager();
		}
		return instance;
	}

	~OsgManager();

	//bool loadObjectModel(const char* filename, const char* basepath, osgViewer::Viewer* pViewer);

	void setViewer(osgViewer::Viewer& viewer);
	void test();
	std::string createSpheres(const osg::Vec3& pos);
#ifdef PHYSICS_ON
	void updateDynamicActors(const std::string& id, const physx::PxVec3& pxpos);
#endif
	void removeDynamicActors(const std::string& id);
	void showFileModel(const char* pName, bool bPointsOnly);
	void showModelScene(const char* pName);
	bool Collaborate(const char* pName);
	void switchScene();
	void clear();
	void setNormalMap(osg::ref_ptr<osg::Node> geomNode, osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene);
	// slots
	void updateTerrain(Arrays surface, Vector3i pos);
	void reconstructFrame(const std::string& sFilePath);
	bool reconstructFrameStep(std::string sFilePath);
public:
	bool bReload = true;
	TerrainBrush terrainBrush;
	int modifyType;
	VoxelBrush* brush;
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
	osg::ref_ptr<osg::Geometry> getModelGeometry(const char* pName, bool bPointsOnly = false);
	osg::ref_ptr<osg::Node> getModelNode(const char* pName, bool bPointsOnly = false, osg::Vec4 color = osg::Vec4(1.0f, 1.0f, 1.0f, 0.5f));
	osg::ref_ptr<osgShadow::ShadowedScene> getShadowScene(osg::ref_ptr<osg::Geometry> geomNode, ShowType type = ShowType::SHOW_FRONT_AND_BACK);

	void showModelWithShader(osg::ref_ptr<osg::Geometry> pGeom);
private:
	static OsgManager* instance;

	osg::ref_ptr<osg::Group> root = nullptr;
	osg::ref_ptr<osg::Group> rootWireTerrain = nullptr;
	osg::ref_ptr<osg::Group> sunLight = nullptr;
	osg::ref_ptr<osg::Group> rootGeomTerrain = nullptr;
	osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
	osg::ref_ptr<osg::Switch> sceneSwitch;
	osg::BoundingSphere bs;

	bool bFirstTime = false;
	int sceneIdx = 0;
	int sceneMaxIdx = 0;

	//Reconstruct* precon = nullptr;

	std::unordered_map<Vector3i, std::pair<osg::ref_ptr<osg::Geometry>, osg::ref_ptr<osg::Geometry>>, Vector3iHash> sceneMap;

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

