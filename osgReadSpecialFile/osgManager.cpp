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

	osg::Geode *pGeode = pNode->asGeode();
	osg::Geode *pGeodeWireframe = new osg::Geode;
	if (pGeode) {
		size_t nNumChildren = pGeode->getNumChildren();
		for (size_t i = 0; i < nNumChildren; ++i) {
			osg::Node *pChildNode = pGeode->getChild(i);
			osg::Geometry *pChildGeom = pChildNode->asGeometry();
			if (pChildGeom) {
				osg::ref_ptr<osg::Geometry> geomWireframe = new osg::Geometry(*pChildGeom, osg::CopyOp::DEEP_COPY_STATESETS);
				setWireFrame(geomWireframe->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);

				pGeodeWireframe->addDrawable(geomWireframe);
			}
		}
	}

	m_pMeshGroup->addChild(pNode);
	m_pSceneSwitcher->addChild(pGeodeWireframe);
}

void OsgManager::transfer() {
	std::string sFileName = "../data/ply_ext/scene_dense_mesh_03-02-01.ply";
	osg::Node* pNode = readPlySpecial(sFileName);

	osgDB::writeNodeFile(*pNode, std::string("./tmp.osgb"));
}
