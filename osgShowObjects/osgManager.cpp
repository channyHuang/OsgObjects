#include "osgManager.h"

#include <filesystem>

#include "commonOsg/commonOsg.h"
#include "readObjTinyobj.h"

OsgManager* OsgManager::m_pInstance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
	m_pRootGeomDistance = new osg::Group;
	m_pSceneSwitcher->addChild(m_pRootGeomDistance);
}

OsgManager::~OsgManager() {
	m_pRootGeomDistance.release();
}

void OsgManager::readObjTinyobj(const std::string &sFileName, const std::string& sBasePath, bool bUseOsgLoad) {
	osg::ref_ptr<osg::Geometry> pGeom = getObjTinyobj(sFileName.c_str(), sBasePath.c_str());
	m_pSceneSwitcher->addChild(pGeom);
}

void OsgManager::obj2osgt(const std::string& str) {
	osg::ref_ptr<osg::Geometry> geom = getObjTinyobj(str.c_str(), "");
	osgDB::writeNodeFile(*geom, std::string("./hud.osgt"));
}
