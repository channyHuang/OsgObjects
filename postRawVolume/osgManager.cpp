#include "osgManager.h"

#include "commonOsg/commonOsg.h"

OsgManager* OsgManager::instance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
	m_pRootGeomDistance = new osg::Group;
	m_pSceneSwitcher->addChild(m_pRootGeomDistance);
}

OsgManager::~OsgManager() {
	m_pRootGeomDistance.release();
}

void OsgManager::updateShow(int nType) {
	
}
