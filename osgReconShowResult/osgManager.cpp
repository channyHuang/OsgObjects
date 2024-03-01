#include "osgManager.h"

#include "osgPickHandler.h"

#include <filesystem>

#include <opencv2/highgui/highgui.hpp>

OsgManager* OsgManager::instance = nullptr;

OsgManager::OsgManager() {
	root = new osg::Group;
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

	sceneSwitch = new osg::Switch;
}

OsgManager::~OsgManager() {
	clear();
	pviewer.release();
}

void OsgManager::setViewer(osgViewer::Viewer& viewer) {
	pviewer = &viewer;
	init();

	pviewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
	eye = osg::Vec3(0, 0, 1);
	pviewer->getCamera()->setViewMatrixAsLookAt(eye, center, up);
}

void OsgManager::init() {
	clear();
	pviewer->addEventHandler(new PickHandler());

	root->addChild(createSkyBox());
	root->addChild(createAxis());
	root->addChild(sceneSwitch);

	bs = osg::BoundingSphere(osg::Vec3(0.f, 0.f, 0.f), 1.f);
}

void OsgManager::clear() {
	sceneSwitch->removeChildren(0, sceneSwitch->getNumChildren());
	bs = osg::BoundingSphere(osg::Vec3(0.f, 0.f, 0.f), 1.f);
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

std::set<std::string> getImageList(const std::string& sPath) {
	std::set<std::string> setFileNames;
	for (auto file : std::filesystem::directory_iterator(sPath)) {
		if (file.is_directory()) continue;
		std::string sFileName = file.path().filename().string();
		setFileNames.insert(sFileName);
	}
	return setFileNames;
}

void filterImages(const std::string& sPath, std::set<std::string> setFileNames) {

}

void detectFeatures(const std::string& sPath, std::set<std::string> setFileNames) {
#pragma omp parallel for num_threads(30)
	for (auto file : setFileNames) {
		std::string sFullName = sPath + "/" + file;
		// detect feature using SiftGPU
	}
}

void featureMatch(const std::string& sPath, std::set<std::string> setFileNames) {
#pragma omp parallel for num_threads(30)
	for (auto itr = setFileNames.begin(); itr != setFileNames.end(); itr++) {
		auto nitr = itr;
		nitr++;
		if (nitr == setFileNames.end()) break;
		// match seq
	}
}

void estimate() {

}

void OsgManager::dosomething() {
	std::string sImg1 = "", sImg2 = "";
	std::string sFeat1 = "", sFeat2 = "";
	float x, y, sigma, others;
	int numOfMatches, imgIdx1, imgIdx2;
	int idx1, idx2;
	std::unordered_map<int, std::pair<float, float>> features1, features2;

	std::ifstream ifs(sFeat1);
	int idx = 0;
	while (ifs.eof()) {
		ifs >> x >> y >> sigma >> others;
		features1[idx++] = std::make_pair(x, y);
	}

	ifs.open(sFeat2);
	idx = 0;
	while (ifs.eof()) {
		ifs >> x >> y >> sigma >> others;
		features2[idx++] = std::make_pair(x, y);
	}

	std::string sMatchs = "matches.f.txt";
	ifs.open(sMatchs);
	while (ifs.eof()) {
		ifs >> imgIdx1 >> imgIdx2 >> numOfMatches;
		for (int i = 0; i < numOfMatches; ++i) {
			ifs >> idx1 >> idx2;
		}
	}

}
