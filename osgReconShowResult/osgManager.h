#pragma once

#include <unordered_map>

#include "commonOsg/commonOsg.h"
#include "commonMath/vector3i.h"

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

	void setViewer(osgViewer::Viewer& viewer);
	void switchScene();
	void clear();

	void dosomething();

public:
	bool bReload = true;
	bool valid = false;
	float progress = 0.f;
private:
	OsgManager();
	void init();

private:
	static OsgManager* instance;

	osg::ref_ptr<osg::Group> root = nullptr;
	osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
	osg::ref_ptr<osg::Switch> sceneSwitch;
	osg::BoundingSphere bs;

	bool bFirstTime = false;
	int sceneIdx = 0;
	int sceneMaxIdx = 0;

	osg::Vec3 up, eye, center;
	float theta = 0, phi = 0;

	std::mutex mutex;
};

