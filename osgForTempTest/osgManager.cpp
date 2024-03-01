#include "osgManager.h"

#include <filesystem>

OsgManager* OsgManager::instance = nullptr;

OsgManager::OsgManager() {
	root = new osg::Group;
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	root->addChild(createAxis());

	sceneSwitch = new osg::Switch;
	sceneSwitch->setAllChildrenOn();
	root->addChild(sceneSwitch);

	rootWire = new osg::Group;
	rootGeom = new osg::Group;
	sceneSwitch->addChild(rootWire);
	sceneSwitch->addChild(rootGeom);
}

OsgManager::~OsgManager() {
	pviewer.release();
}

void OsgManager::setViewer(osgViewer::Viewer& viewer) {
	pviewer = &viewer;

	pviewer->addEventHandler(new PickHandler());
	pviewer->setSceneData(root);
}

void OsgManager::switchScene() {
	sceneMaxIdx = sceneSwitch->getNumChildren();
	if (sceneIdx >= sceneMaxIdx) {
		sceneSwitch->setAllChildrenOn();
		sceneIdx = 0;
	}
	else {
		sceneSwitch->setSingleChildOn(sceneIdx);
		sceneIdx++;
	}
}
