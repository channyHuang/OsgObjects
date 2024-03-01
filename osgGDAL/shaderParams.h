#pragma once

#include <string>

#include <osgViewer/Viewer>
#include <osg/Group>
#include <osg/Switch>
#include <osg/Program>
#include <osgDB/ReadFile>
#include <osg/LineWidth>
#include <osgViewer/Renderer>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/BufferIndexBinding>

#include "commonOsg/commonOsg.h"

class ShaderParams {
public:
	static ShaderParams* getInstance() {
		if (instance == nullptr) {
			instance = new ShaderParams();
		}
		return instance;
	}

	~ShaderParams() {};

	void show_01(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer);
	void show_02(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer);
	void show_02_1(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer);
	void show_02_2(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer);
	void show_03(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer);

private:
	ShaderParams() {};

	static ShaderParams* instance;

	std::string sPath = "./../bin/shader-cookbook/chapter";
};
