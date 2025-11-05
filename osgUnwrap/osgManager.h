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

#include "commonOsg/osgManagerBase.h"

class OsgManager : public OsgManagerBase
{
public:
	static OsgManager* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new OsgManager();
		}
		return m_pInstance;
	}

	virtual ~OsgManager();

	GLuint genTexture(const unsigned char* data, int width, int height);

protected:
	static OsgManager* m_pInstance;
	
protected:
	OsgManager();

	osg::ref_ptr<osg::Group> root = nullptr;
	osg::ref_ptr<osg::Group> rootWireTerrain = nullptr;
	osg::ref_ptr<osg::Group> sunLight = nullptr;
	osg::ref_ptr<osg::Group> rootGeomTerrain = nullptr;
	osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
	osg::ref_ptr<osg::Switch> sceneSwitch;
};

