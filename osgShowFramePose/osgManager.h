#pragma once

#include <unordered_map>
#include <mutex>

#include <osgViewer/Viewer>
#include <osg/Group>
#include <osg/Switch>
#include <osg/Program>
#include <osgDB/ReadFile>
#include <osg/LineWidth>
#include <osgViewer/Renderer>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/MatrixTransform>

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

	void setViewer(osgViewer::Viewer& viewer);
	//void shader(osg::ref_ptr<osg::Geometry> pGeom, std::string sImgName);

	void show2Camera();
	void showCamera(std::string sPath, std::string sImageFile, bool selected = false);
	void showPointAndCamera();

	bool combinePose(osg::Matrix& combMat, std::string sPath, std::string sFile1, std::string sFile2);
	void combineCamera();

	void combineAndWrite();
	
	void showGrid(osg::Vec3f center = osg::Vec3f(0, 0, 0), float side = 10.f);
	void showFrame();
	
private:
	OsgManager();
	
private:
	static OsgManager* instance;

	osg::ref_ptr<osg::Group> root = nullptr;
	osg::ref_ptr<osg::Group> rootWireTerrain = nullptr;
	osg::ref_ptr<osg::Group> sunLight = nullptr;
	osg::ref_ptr<osg::Group> rootGeomTerrain = nullptr;
	osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
	osg::ref_ptr<osg::Switch> sceneSwitch;
};

