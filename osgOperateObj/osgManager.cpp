#include "osgManager.h"

#include "osgPickHandler.h"

#include <filesystem>

#include <vcg/complex/allocate.h>
#include <vcg/complex/algorithms/pointcloud_normal.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/export_obj.h>
#include <wrap/io_trimesh/export_ply.h>
#include <wrap/ply/plylib.cpp>
using namespace vcg;

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
	} else {
		sceneSwitch->setSingleChildOn(sceneIdx);
		sceneIdx++;
	}
}

class MyEdge;
class MyFace;
class MyVertex;
struct MyUsedTypes : public UsedTypes<	Use<MyVertex>   ::AsVertexType,
	Use<MyEdge>     ::AsEdgeType,
	Use<MyFace>     ::AsFaceType> {};

class MyVertex : public Vertex<MyUsedTypes, vertex::Coord3f, vertex::Normal3f, vertex::BitFlags  > {};
class MyFace : public Face< MyUsedTypes, face::FFAdj, face::VertexRef, face::BitFlags > {};
class MyEdge : public Edge<MyUsedTypes> {};
class MyMesh : public tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge>  > {};
typedef typename MyMesh::VertexType::CoordType   Point3x;

#include <cstring>
#include <string>

void OsgManager::calcObjNormal(const std::string& sPath, std::string sOutPath) {
	//std::string sPath = "E:/pythonProjects/puma-lidar-modify/data/mapall/ply/sequences/00/origin/";
	//std::string sPath = "D:/dataset/lab/lab_level_pcd/mapall_frame/";
	//std::string sOutPath = "E:/pythonProjects/puma-lidar-modify/data/mapall/ply/sequences/00/velodyne/";
	MyMesh vcgMesh;
	for (auto& fileName : std::filesystem::directory_iterator(sPath)) {
		if (fileName.is_directory()) continue;

		std::string sNameOnly = fileName.path().filename().string();
		std::string sNameWithoutType = fileName.path().root_name().string();

		std::string type = fileName.path().extension().string();
		if (std::strcmp(type.c_str(), "obj") != 0) continue;

		std::string sFullName = sPath + sNameOnly;

		char sOutName[10] = { 0 };
		int nOutName = std::stoi(sNameWithoutType);
		sprintf(sOutName, "%03d.ply", nOutName);
		
		std::string sOutFullName = sOutPath + std::string(sOutName);
		vcgMesh.Clear();

		int loadmask;
		tri::PointCloudNormal<MyMesh>::Param par;
		par.fittingAdjNum = 10;
		par.useViewPoint = true;

		vcg::tri::io::ImporterOBJ<MyMesh>::Open(vcgMesh, sFullName.c_str(), loadmask);
		tri::PointCloudNormal<MyMesh>::Compute(vcgMesh, par);
		//tri::io::ExporterOBJ<MyMesh>::Save(vcgMesh, sOutName.c_str(), tri::io::Mask::IOM_VERTNORMAL);
		tri::io::ExporterPLY<MyMesh>::Save(vcgMesh, sOutFullName.c_str(), tri::io::Mask::IOM_VERTNORMAL);
	}
}

// optimized_pose.txt
std::vector<osg::Matrix> getPose(const std::string& sPath, std::string sPoseName = "optimized_pose.txt") {
	std::string sFullPoseName = sPath + sPoseName;
	std::vector<osg::Matrix> trans;
	std::string line;
	if (std::strcmp(sPoseName.c_str(), "optimized_pose.txt") == 0) {
		std::ifstream ifs(sFullPoseName);
		while (!ifs.fail() && !ifs.eof()) {
			std::getline(ifs, line);
			if (line.length() < 8) continue;
			std::vector<std::string> res = splitString(line, ' ');

			osg::Matrix tr = osg::Matrix::rotate(osg::Quat(std::atof(res[4].c_str()), std::atof(res[5].c_str()), std::atof(res[6].c_str()), std::atof(res[7].c_str())));
			tr = tr.translate(osg::Vec3(std::atof(res[1].c_str()), std::atof(res[2].c_str()), std::atof(res[3].c_str())));

			trans.push_back(tr);
		}
		ifs.close();
	}
	else {
		std::ifstream ifs(sFullPoseName);
		std::getline(ifs, line);
		while (!ifs.fail() && !ifs.eof()) {
			std::getline(ifs, line);
			if (line.length() < 18) continue;
			std::vector<std::string> res = splitString(line, ',');
			if (res.size() < 18) continue;
			osg::Matrix tr;
			int idx = 2;
			for (int x = 0; x < 4; x++) {
				for (int y = 0; y < 4; ++y) {
					tr(x, y) = std::atof(res[idx++].c_str());
				}
			}
			trans.push_back(tr);
		}
		ifs.close();
	}
	return trans;
}

void OsgManager::combineAllPointCloudObj(const std::string& sPath, const std::string& sOutPath, const std::string& sOutName) {
	std::ofstream ofs(sOutPath + "/" + sOutName);
	unsigned int count = 0, offset = 0;
	for (auto& file : std::filesystem::directory_iterator(sPath)) {
		if (file.is_directory()) continue;

		std::string ext = file.path().extension().string();
		if (std::strcmp(ext.c_str(), "obj") != 0) continue;

		std::string name = file.path().filename().string();

		std::ifstream ifs(sPath + "/" + name);
		std::string line;
		while (!ifs.eof()) {
			std::getline(ifs, line);

			if (line.length() <= 5) continue;
			if (line[0] == '#') continue;
			if (line[0] == 'v' && line[1] == ' ') {
				ofs << line << std::endl;
				count++;
			}
			else if (line[0] == 'f') {
				std::vector<std::string> res = splitString(line, ' ');
				if (res.size() < 4) continue;
				ofs << "f " << atoi(res[1].c_str()) + offset << " " << atoi(res[2].c_str()) + offset << " " << atoi(res[3].c_str()) + offset << std::endl;
			}
		}
		offset += count;
	}
	ofs.close();
}

void OsgManager::pcd2obj(const std::string& sPath, const std::string& sOutPath) {
	for (auto& file : std::filesystem::directory_iterator(sPath)) {
		if (file.is_directory()) continue;

		std::string ext = file.path().extension().string();
		if (std::strcmp(ext.c_str(), "pcd") != 0) continue;

		std::string name = file.path().filename().string();

		std::ifstream ifs;
		ifs.open(name, std::ios::binary | std::ios::in);
		std::string line;
		for (int i = 0; i < 10; ++i) {
			std::getline(ifs, line);
			std::cout << line.c_str() << std::endl;
		}

		std::vector<std::string> res = splitString(line, ' ');
		int numOfPoints = std::atoi(res[1].c_str());

		std::getline(ifs, line);

		std::ofstream ofs;
		ofs.open(sOutPath + "/" + name + ".obj");
		for (int i = 0; i < numOfPoints; ++i) {
			float data[8];
			char others[128] = { 0 };
			for (int j = 0; j < 3; ++j)
				ifs.read((char*)&data[j], sizeof(float));
			ifs.read((char*)&others, sizeof(char) * 11);

			ofs << "v " << data[0] << " " << data[1] << " " << data[2] << std::endl;
		}
		ofs.close();
	}
}

void OsgManager::combineAllObj(const std::string& sPath, std::string sOutPath, std::string sOutName) {
	std::string sPoseName = sPath + "optimized_pose.txt";
	std::vector<osg::Matrix> vMatTranslate = getPose(sPoseName);

	int index = 0;
	
	for (auto& fileName : std::filesystem::directory_iterator( sPath )) {
		if (fileName.is_directory()) continue;
		
		std::string fix = fileName.path().extension().string();
		if (std::strcmp(fix.c_str(), "obj") != 0) continue;

		std::string baseName = fileName.path().root_name().string();
		int nameIdx = std::atoi(baseName.c_str());

		char cIndex[10] = { 0 };
		int baseIndex = std::stoi(baseName);
		sprintf(cIndex, "%03d", baseIndex);

		osg::ref_ptr<osg::Geometry> pGeom = createGeomUsingTinyobj(fileName.path().root_name().string().c_str(), "");

		osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();
		osg::Matrix tr = vMatTranslate[nameIdx];
		std::ofstream ofs(sOutPath + cIndex + ".obj");
		for (int i = 0; i < varray->size(); ++i) {

			osg::Vec3 vRotPoint = tr.preMult(varray->at(i));

			ofs << "v " << vRotPoint.x() << " " << vRotPoint.y() << " " << vRotPoint.z() << std::endl;
		}
		index++;
		ofs.close();
	}
}

void OsgManager::transformObj(const std::string& sPath, std::string sPoseName, std::string sOutPath) {
	std::string sFullPoseName = sPath + sPoseName;
	std::vector<osg::Matrix> vMatTranslate = getPose(sPath, sPoseName);
	int fileIdx = 0;
	for (const auto& fileName : std::filesystem::directory_iterator( sPath )) {
		if (fileName.is_directory()) continue;

		std::string fix = fileName.path().extension().string();
		if (std::strcmp(fix.c_str(), "obj") != 0) continue;

		std::string baseName = fileName.path().root_name().string();
		
		std::string sFileIdx = fileName.path().filename().string().substr(10, 10);
		fileIdx = std::atoi(sFileIdx.c_str());
		
		char cIndex[10] = { 0 };
		sprintf(cIndex, "%03d", fileIdx);

		osg::ref_ptr<osg::Geometry> pGeom = createGeomUsingTinyobj(fileName.path().root_name().string().c_str(), "");

		osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();
		osg::Matrix tr = vMatTranslate[fileIdx++];
		std::ofstream ofs(sOutPath + std::string(cIndex) + ".obj");
		for (int i = 0; i < varray->size(); ++i) {

			osg::Vec3 vRotPoint = tr.preMult(varray->at(i));

			ofs << "v " << vRotPoint.x() << " " << vRotPoint.y() << " " << vRotPoint.z() << std::endl;
		}
		ofs.close();
	}
}
