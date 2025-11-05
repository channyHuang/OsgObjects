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

void OsgManager::showBoxWithTexture() {
	HGVertex hv;
	osg::Node *pNode = hv.showBoxWithTexture();
	m_pSceneSwitcher->addChild(pNode);

	osg::LightSource *pLight = hv.light();
	m_pRootGeomDistance->addChild(pLight);
}

void OsgManager::showBoxWithMultiTexture() {
	HGVertex hv;
	osg::Node *pNode = hv.showBoxWithMultiTexture();
	m_pSceneSwitcher->addChild(pNode);

	osg::LightSource *pLight = hv.light();
	m_pRootGeomDistance->addChild(pLight);
}

void OsgManager::showBoxWithRightTexture() {
	HGVertex hv;
	osg::Node *pNode = hv.showBoxWithRightTexture();
	m_pSceneSwitcher->addChild(pNode);

	osg::LightSource *pLight = hv.light();
	m_pRootGeomDistance->addChild(pLight);
}
