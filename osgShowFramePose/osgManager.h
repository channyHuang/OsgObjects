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

	//void shader(osg::ref_ptr<osg::Geometry> pGeom, std::string sImgName);

	void show2Camera();
	void showCamera(std::string sPath, std::string sImageFile, bool selected = false);
	void showPointAndCamera();

	bool combinePose(osg::Matrix& combMat, std::string sPath, std::string sFile1, std::string sFile2);
	void combineCamera();

	void combineAndWrite();
	
	void showGrid(osg::Vec3f center = osg::Vec3f(0, 0, 0), float side = 10.f);
	void showFrame();

protected:
	static OsgManager* m_pInstance;

protected:
	OsgManager();

	osg::ref_ptr<osg::Group> m_pMeshGroup = nullptr;
};

