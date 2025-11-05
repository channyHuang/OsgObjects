#include "osgManager.h"

#include "commonOsg/commonOsg.h"

OsgManager* OsgManager::m_pInstance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
	// m_pMeshGroup = new osg::Group;
	// m_pSceneSwitcher->addChild(m_pMeshGroup);
}

OsgManager::~OsgManager() {
	// m_pMeshGroup.release();
}
