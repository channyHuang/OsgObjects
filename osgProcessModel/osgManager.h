#pragma once

#include <unordered_map>
#include <mutex>

#include "commonOsg/osgManagerBase.h"
#include "commonOsg/commonOsg.h"

class OsgManager: public OsgManagerBase {
public:
	static OsgManager* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new OsgManager();
		}
		return m_pInstance;
	}

	virtual ~OsgManager();

	void calcObjNormal(const std::string& sPath, std::string sOutPath = "");
	void combineAllPointCloudObj(const std::string& sPath, const std::string& sOutPath, const std::string& sOutName);
	void pcd2obj(const std::string& sPath, const std::string& sOutPath);
	void combineAllObj(const std::string& sPath, std::string sOutPath, std::string sOutName);
	void transformObj(const std::string& sPath, std::string sPoseName, std::string sOutPath = "");
	void switchScene();
	void clear();
	void setNormalMap(osg::ref_ptr<osg::Node> geomNode, osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene);

public:
	bool bReload = true;
	bool valid = false;
	float progress = 0.f;

protected:
	OsgManager();
	void init();

private:
	static OsgManager* m_pInstance;

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

