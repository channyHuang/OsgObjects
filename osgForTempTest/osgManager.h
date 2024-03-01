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

#include "commonOsg/osgPickHandler.h"
#include "commonOsg/commonOsg.h"

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
	void switchScene();

private:
	OsgManager();
	
private:
	static OsgManager* instance;

	PickHandler* picker = nullptr;

	osg::ref_ptr<osg::Group> root = nullptr;
	osg::ref_ptr<osg::Group> rootWire = nullptr;
	osg::ref_ptr<osg::Group> sunLight = nullptr;
	osg::ref_ptr<osg::Group> rootGeom = nullptr;
	osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
	osg::ref_ptr<osg::Switch> sceneSwitch;
	int sceneMaxIdx = 0, sceneIdx = 0;
};

