#include "osgManager.h"

#include "osgPickHandler.h"

#include "shaderParams.h"

OsgManager* OsgManager::instance = nullptr;

OsgManager::OsgManager() {
	root = new osg::Group;
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	root->addChild(createAxis());

	sceneSwitch = new osg::Switch;
	sceneSwitch->setAllChildrenOn();
}

OsgManager::~OsgManager() {
	pviewer.release();
}

void OsgManager::setViewer(osgViewer::Viewer& viewer) {
	pviewer = &viewer;

	pviewer->addEventHandler(new PickHandler());
	pviewer->setSceneData(root);
}
