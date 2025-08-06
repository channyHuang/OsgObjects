#include "GLManager.h"
#include "osgManager.h"

#include <filesystem>

OsgManager* OsgManager::m_pInstance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
	m_pRootGeomDistance = new osg::Group;
	m_pSceneSwitcher->addChild(m_pRootGeomDistance);
}

OsgManager::~OsgManager() {
	m_pRootGeomDistance.release();
}

void OsgManager::updateShow(int nShowType) {
	GLManager::getInstance()->draw_frame_start();
	printf("%s[%d] nShowType = %d\n", __FILE__, __LINE__, nShowType);
	switch (nShowType) {
	case 0:
		GLManager::getInstance()->setPts();
		GLManager::getInstance()->draw((EDrawMode)nShowType);
		break;
	case 1:
		GLManager::getInstance()->setPts();
		GLManager::getInstance()->draw((EDrawMode)nShowType);
		break;
	case 2:
		GLManager::getInstance()->setPts();
		GLManager::getInstance()->draw((EDrawMode)nShowType);
		break;
	case 3:
		GLManager::getInstance()->setPts();
		GLManager::getInstance()->draw((EDrawMode)nShowType);
		break;
	default:
		break;
	}
}