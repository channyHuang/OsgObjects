#include "osgManager.h"

#include "commonOsg/commonOsg.h"

#include "plySpecial/ReaderWriterPLY.hpp"

OsgManager* OsgManager::m_pInstance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
	m_pMeshGroup = new osg::Group;
	m_pSceneSwitcher->addChild(m_pMeshGroup);
}

OsgManager::~OsgManager() {
	m_pMeshGroup.release();
}

void OsgManager::loadSpecialPly() {
	std::string sFileName = "../data/ply_ext/scene_dense_mesh_03-02-01.ply";
	osg::Node* pNode = readPlySpecial(sFileName);
	m_pMeshGroup->addChild(pNode);
}