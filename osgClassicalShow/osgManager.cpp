#include "osgManager.h"

#include "commonOsg/commonOsg.h"

#include "HGVertex.h"

OsgManager* OsgManager::m_pInstance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
	m_pRootGeomDistance = new osg::Group;
	m_pSceneSwitcher->addChild(m_pRootGeomDistance);
}

OsgManager::~OsgManager() {
	m_pRootGeomDistance.release();
}

void OsgManager::showBox() {
	HGVertex hv;
	osg::Node *pNode = hv.originBox();
	m_pSceneSwitcher->addChild(pNode);

	osg::LightSource *pLight = hv.light();
	m_pRootGeomDistance->addChild(pLight);
}
