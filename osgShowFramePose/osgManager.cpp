#include "osgManager.h"

#include "osgPickHandler.h"

#include "commonOsg/commonOsg.h"

OsgManager* OsgManager::instance = nullptr;

#include <unordered_map>

#include "commonMath/vector2.h"
#include "commonMath/vector3.h"
#include "commonMath/triangle.h"
#include "commonGeometry/delaunay.h"

class CommonPlane {
public:
	CommonPlane() : vNormal_(Vector3(0, 1, 0)), vPoint_(Vector3(0)) {}
	CommonPlane(const Vector3& _vNormal, const Vector3& _vPoint) : vNormal_(_vNormal), vPoint_(_vPoint) {}

	Vector3 vNormal_;
	Vector3 vPoint_;
};

osg::ref_ptr<osg::Geode> createCamera(float scale = 1.f) {
	osg::ref_ptr<osg::Geode> geode(new osg::Geode());
	osg::ref_ptr<osg::Geometry> geomLocation(new osg::Geometry());
	osg::ref_ptr<osg::Geometry> geomCamera(new osg::Geometry());

	osg::Vec3Array* vertices(new osg::Vec3Array());
	vertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
	vertices->push_back(osg::Vec3(scale, scale, scale));
	vertices->push_back(osg::Vec3(scale, -scale, scale));
	vertices->push_back(osg::Vec3(-scale, -scale, scale));
	vertices->push_back(osg::Vec3(-scale, scale, scale));
	geomLocation->setVertexArray(vertices);
	geomCamera->setVertexArray(vertices);

	osg::Vec4Array* colorsLocation(new osg::Vec4Array());
	colorsLocation->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 0.5f));
	geomLocation->setColorArray(colorsLocation, osg::Array::BIND_OVERALL);

	osg::Vec4Array* colorsCamera(new osg::Vec4Array());
	colorsCamera->push_back(osg::Vec4(0.0f, 1.0f, 1.0f, 0.5f));
	geomCamera->setColorArray(colorsCamera, osg::Array::BIND_OVERALL);

	std::vector<unsigned int> vindicesLocation = {0, 1, 0, 2, 0, 3, 0, 4};
	geomLocation->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, vindicesLocation.size(), vindicesLocation.data()));

	std::vector<unsigned int> vindicesCamera = { 1, 2, 3, 2, 3, 4, 3, 4, 1};
	geomCamera->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, vindicesCamera.size(), vindicesCamera.data()));

	osg::PolygonMode* polymode = new osg::PolygonMode;
	polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	geomLocation->getOrCreateStateSet()->setAttributeAndModes(polymode, osg::StateAttribute::ON);
	osg::PolygonMode* polymodeCamera = new osg::PolygonMode;
	polymodeCamera->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
	geomCamera->getOrCreateStateSet()->setAttributeAndModes(polymodeCamera, osg::StateAttribute::ON);

	geode->addDrawable(geomCamera);
	geode->addDrawable(geomLocation);

	osg::ref_ptr<osg::LineWidth> linewidth = getNewLineWidth(2.f);
	geode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);

	return geode;
}

OsgManager::OsgManager() {
	root = new osg::Group;
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	root->addChild(createAxis(3.f));
}

OsgManager::~OsgManager() {
	pviewer.release();
}

void OsgManager::setViewer(osgViewer::Viewer& viewer) {
	pviewer = &viewer;

	pviewer->addEventHandler(new PickHandler());
	pviewer->setSceneData(root);
}

void OsgManager::showCamera(std::string sPath, std::string sImageFile, bool selected) {
	//std::string sImageFile = "E:/projects/instant-ngp/data/nerf/lab_sub/colmap_text/images.txt";
	std::ifstream ifs(sPath + "/" + sImageFile + "/images.txt");
	std::string line;
	int lineIdx = 0;
	while (!ifs.eof()) {
		std::getline(ifs, line);
		if (line.length() <= 0) continue;
		if (line[0] == '#') continue;
		lineIdx++;
		if ((lineIdx & 1) == 1) {
			std::vector<std::string> params = splitString(line, ' ');
			if (params.size() < 9) {
				std::cout << "error " << std::endl;
				continue;
			}
			osg::Quat q;
			for (int i = 0; i < 4; ++i) {
				q[(i + 3) % 4] = std::stof(params[1 + i]);
			}
			osg::Vec3 t;
			for (int i = 0; i < 3; ++i) {
				t[i] = std::stof(params[5 + i]);
			}

			if (selected) {
				std::string name = params[9];
				if ((std::strcmp("IMG_0089.JPG", name.c_str()) != 0) && (std::strcmp("IMG_0097.JPG", name.c_str()) != 0)) {
					continue;
				}
			}

			osg::Matrix rot;
			rot.makeIdentity();
			rot.setRotate(q);

			osg::Matrix invRot = osg::Matrix::inverse(rot);

			osg::Matrix trans;
			trans.makeIdentity();

			trans.setRotate(invRot.getRotate());
			trans.setTrans((-t) * invRot);

			osg::ref_ptr<osg::MatrixTransform> pTrans = new osg::MatrixTransform;
			pTrans->setMatrix(trans);
			pTrans->addChild(createAxis(0.5f));
			pTrans->addChild(createCamera());

			root->addChild(pTrans);
		}
	}
}

void OsgManager::showGrid(osg::Vec3f center, float side) {
	osg::ref_ptr<osg::Geometry> geom(new osg::Geometry());

	float half = side / 2.f;
	osg::Vec3 stPos = center - osg::Vec3(half, half, half);
	osg::Vec3 endPos = center + osg::Vec3(half, half, half);

	osg::Vec3Array* vertices(new osg::Vec3Array());

	for (int i = stPos.x(); i <= endPos.x(); ++i) {
		for (int j = stPos.y(); j <= endPos.y(); ++j) {
			vertices->push_back(osg::Vec3(i, j, stPos.z()));
			vertices->push_back(osg::Vec3(i, j, endPos.z()));
		}

		for (int k = stPos.z(); k <= endPos.z(); ++k) {
			vertices->push_back(osg::Vec3(i, stPos.y(), k));
			vertices->push_back(osg::Vec3(i, endPos.y(), k));
		}
	}

	for (int j = stPos.y(); j <= endPos.y(); ++j) {
		for (int k = stPos.z(); k <= endPos.z(); ++k) {
			vertices->push_back(osg::Vec3(stPos.x(), j, k));
			vertices->push_back(osg::Vec3(endPos.x(), j, k));
		}
	}

	geom->setVertexArray(vertices);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, vertices->size()));

	osg::Vec4Array* colors(new osg::Vec4Array());
	colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	geom->setColorArray(colors, osg::Array::BIND_OVERALL);

	root->addChild(geom);
}

void OsgManager::show2Camera() {

	osg::Matrix trans;
	trans.makeIdentity();

	osg::ref_ptr<osg::MatrixTransform> pTrans = new osg::MatrixTransform;
	pTrans->setMatrix(trans);
	pTrans->addChild(createAxis());
	pTrans->addChild(createCamera());

	root->addChild(pTrans);
}

class ImageSt {
public:
	int id;
	osg::Quat q;
	osg::Vec3 t;
	osg::Matrix invRot;
	osg::Vec3 invT;
	osg::Matrix trans;
	std::string name;
	int idCamera;
	std::unordered_map<int, Vector2> points2d;
	std::vector<Vector2> points2dOnly;

	ImageSt() {}
	ImageSt(int _id, osg::Quat _q, osg::Vec3 _t, std::string _name = "", int _idCamera = -1) : q(_q), t(_t), name(_name) {
		osg::Matrix rot;
		rot.makeIdentity();
		rot.setRotate(q);

		invRot = osg::Matrix::inverse(rot);
		invT = (-t) * invRot;

		trans.makeIdentity();
		trans.setRotate(invRot.getRotate());
		trans.setTrans(invT);

		id = _id;
		name = _name;
		idCamera = _idCamera;
	}
};

class Point3D {
public:
	int id;
	Vector3 pos;
	Vector3i color;
	float error;
	std::vector<std::pair<int, int>> idImgs;
};

struct Point2D {
	int id;
	float x, y;
};

std::unordered_map<int, ImageSt> loadImages(std::string sImageFile = "E:/projects/instant-ngp/data/nerf/lab_sub/colmap_text/images.txt") {
	std::unordered_map<int, ImageSt> images;
	std::ifstream ifs(sImageFile);
	std::string line;
	int lineIdx = 0;
	while (!ifs.eof()) {
		std::getline(ifs, line);
		if (line.length() <= 0) continue;
		if (line[0] == '#') continue;
		
		std::vector<std::string> params = splitString(line, ' ');
		int idImg = std::stoi(params[0]);
		std::string name = params[9];
		int idCamera = std::stoi(params[8]);

		osg::Quat q;
		for (int i = 0; i < 4; ++i) {
			q[(i + 3) % 4] = std::stof(params[1 + i]);
		}
		osg::Vec3 t;
		for (int i = 0; i < 3; ++i) {
			t[i] = std::stof(params[5 + i]);
		}

		images[idImg] = ImageSt(idImg, q, t, name, idCamera);
		
		// points2d
		std::getline(ifs, line);
		params = splitString(line, ' ');
		for (int i = 2; i < params.size(); i += 3) {
			int idPoint3d = std::stoi(params[i]);
			if (idPoint3d == -1) {
				images[idImg].points2dOnly.push_back(Vector2(std::stof(params[i - 2]), std::stof(params[i - 1])));
			}
			else {
				images[idImg].points2d[idPoint3d] = Vector2(std::stof(params[i - 2]), std::stof(params[i - 1]));
			}
		}
	}
	ifs.close();
	return images;
}

std::unordered_map<int, Point3D> loadPoint3d(std::string sPointsFile = "E:/projects/instant-ngp/data/nerf/lab_sub/colmap_text/points3D.txt") {
	std::unordered_map<int, Point3D> points3d;

	std::ifstream ifs(sPointsFile);
	std::string line;
	int lineIdx = 0;
	while (!ifs.eof()) {
		std::getline(ifs, line);
		if (line.length() <= 0) continue;
		if (line[0] == '#') continue;

		std::vector<std::string> params = splitString(line, ' ');

		int idPoint3d = std::stoi(params[0]);
		points3d[idPoint3d].id = idPoint3d;
		for (int i = 0; i < 3; ++i) {
			points3d[idPoint3d].pos[i] = std::stof(params[1 + i]);
		}
		for (int i = 0; i < 3; ++i) {
			points3d[idPoint3d].color[i] = std::stoi(params[4 + i]);
		}
		points3d[idPoint3d].error = std::stof(params[7]);
		for (int i = 8; i < params.size(); i += 2) {
			int idImg = std::stoi(params[i]);
			int idPoint2d = std::stoi(params[i + 1]);

			points3d[idPoint3d].idImgs.push_back(std::make_pair(idImg, idPoint2d));
		}
	}
	return points3d;
}

osg::ref_ptr<osg::Geometry> loadPoint3dAsGeom(std::string sPath, std::string sFile, osg::Matrix Rot = osg::Matrix::identity()) {
	std::string sPointsFile = sPath + "/" + sFile + "/points3D.txt";
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	osg::Vec3Array* vertices(new osg::Vec3Array());
	osg::Vec3Array* colors(new osg::Vec3Array());

	std::ifstream ifs(sPointsFile);
	std::string line;
	while (!ifs.eof()) {
		std::getline(ifs, line);
		if (line.length() <= 0) continue;
		if (line[0] == '#') continue;

		std::vector<std::string> params = splitString(line, ' ');
		
		
		osg::Vec3f vertex;
		for (int i = 0; i < 3; ++i) {
			vertex[i] = std::stof(params[1 + i]);
		}
		osg::Vec4 rotVertex = Rot.preMult(osg::Vec4(vertex, 1.f));
		vertices->push_back(osg::Vec3(rotVertex.x(), rotVertex.y(), rotVertex.z()));
		osg::Vec3f color;
		for (int i = 0; i < 3; ++i) {
			color[i] = std::stoi(params[4 + i]) / 256.f;
		}
		colors->push_back(color);
	}
	geom->setVertexArray(vertices);
	geom->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, vertices->size()));
	ifs.close();
	return geom;
}

void OsgManager::showPointAndCamera()
{
	std::string sPath = "E:/projects/instant-ngp/data/nerf/lab_DJ_30_sm";
	std::string sFile = "colmap_text_01";
	showCamera(sPath, sFile);
	osg::ref_ptr<osg::Geometry> geom = loadPoint3dAsGeom(sPath, sFile);
	root->addChild(geom);
}

bool OsgManager::combinePose(osg::Matrix &combMat, std::string sPath, std::string sFile1, std::string sFile2) {
	bool canCombine = false;

	std::unordered_map<int, ImageSt> images0 = loadImages(sPath + "/" + sFile1 + "/images.txt");
	std::unordered_map<int, ImageSt> images1 = loadImages(sPath + "/" + sFile2 + "/images.txt");
	for (auto itr = images1.begin(); itr != images1.end(); itr++) {
		int idImg = itr->second.id;
		if (images0.find(idImg) != images0.end()) {

			canCombine = true;

			osg::Matrix invR1;
			invR1.setRotate(images1[idImg].q);

			osg::Matrix R0(images0[idImg].invRot);

			combMat = R0;
			combMat.preMult(invR1);

			osg::Vec4 tr = -invR1.preMult(osg::Vec4(images1[idImg].invT, 1.f));

			osg::Vec4 tran = R0.preMult(tr);

			combMat.setTrans(osg::Vec3(tran.x(), tran.y(), tran.z()) + images0[idImg].invT);

			break;
		}
	}

	if (canCombine) {
		for (auto itr = images0.begin(); itr != images0.end(); itr++) {
			osg::ref_ptr<osg::MatrixTransform> pTrans = new osg::MatrixTransform;
			pTrans->setMatrix(itr->second.trans);
			pTrans->addChild(createAxis(0.1f));
			pTrans->addChild(createCamera(0.2f));
			root->addChild(pTrans);
		}
		for (auto itr = images1.begin(); itr != images1.end(); itr++) {
			osg::ref_ptr<osg::MatrixTransform> pTrans = new osg::MatrixTransform;
			osg::Matrix combineMat(combMat);
			combineMat.preMult(itr->second.trans);
			pTrans->setMatrix(combineMat);
			pTrans->addChild(createAxis(0.1f));
			pTrans->addChild(createCamera(0.2f));
			root->addChild(pTrans);
		}
	}
	root->addChild(loadPoint3dAsGeom(sPath, sFile1));
	root->addChild(loadPoint3dAsGeom(sPath, sFile2, combMat));

	return canCombine;
}

void OsgManager::combineCamera() {
	std::string sPath = "E:/projects/instant-ngp/data/nerf/lab_DJ_30_sm";
	osg::Matrix combMat;
	bool canCombine = combinePose(combMat, sPath, "colmap_text_1", "colmap_text_0");
	std::cout << "combine " << canCombine << std::endl;
}

void transRt(osg::Quat& q, osg::Vec3& t) {
	osg::Matrix rot;
	rot.setRotate(q);
	rot.setTrans(t);
	q = q.inverse();

	osg::Matrix invq;
	invq.setRotate(q);
	osg::Vec4 tn = -invq.postMult(osg::Vec4(t, 1.0));
	t = osg::Vec3(tn.x(), tn.y(), tn.z());
}

void OsgManager::combineAndWrite() {
	std::string sPath = "E:/projects/instant-ngp/data/nerf/lab_DJ_30_sm";
	std::string sFile1 = "colmap_text_1";
	std::string sFile2 = "colmap_text_0";

	bool canCombine = false;
	osg::Matrix combMat;
	std::unordered_map<int, ImageSt> images0 = loadImages(sPath + "/" + sFile1 + "/images.txt");
	std::unordered_map<int, ImageSt> images1 = loadImages(sPath + "/" + sFile2 + "/images.txt");
	for (auto itr = images1.begin(); itr != images1.end(); itr++) {
		int idImg = itr->second.id;
		if (images0.find(idImg) != images0.end()) {

			canCombine = true;

			osg::Matrix invR1;
			invR1.setRotate(images1[idImg].q);

			osg::Matrix R0(images0[idImg].invRot);

			combMat = R0;
			combMat.preMult(invR1);

			osg::Vec4 tr = -invR1.preMult(osg::Vec4(images1[idImg].invT, 1.f));

			osg::Vec4 tran = R0.preMult(tr);

			combMat.setTrans(osg::Vec3(tran.x(), tran.y(), tran.z()) + images0[idImg].invT);

			break;
		}
	}

	{
		std::unordered_map<int, Point3D> points3d0 = loadPoint3d(sPath + "/" + sFile1 + "/points3D.txt");
		std::unordered_map<int, Point3D> points3d1 = loadPoint3d(sPath + "/" + sFile2 + "/points3D.txt");
		std::ofstream ofs(sPath + "/colmap_text_01/points3D.txt");

		for (auto itr = points3d1.begin(); itr != points3d1.end(); itr++) {
			auto itrPt = points3d0.find(itr->second.id);
			if (itrPt != points3d0.end()) {
				itrPt->second.idImgs.insert(itrPt->second.idImgs.end(), itr->second.idImgs.begin(), itr->second.idImgs.end());
				continue;
			}
			osg::Vec4 vn = combMat.preMult(osg::Vec4(itr->second.pos.x, itr->second.pos.y, itr->second.pos.z, 1.0));
			ofs << itr->second.id << " "
				<< vn.x() << " " << vn.y() << " " << vn.z() << " "
				<< itr->second.color.x << " " << itr->second.color.y << " " << itr->second.color.z << " "
				<< itr->second.error << " ";
			for (auto& item : itr->second.idImgs) {
				ofs << item.first << " " << item.second << " ";
			}
			ofs << std::endl;
		}

		//for (auto itr = points3d0.begin(); itr != points3d0.end(); itr++) {
		//	ofs << itr->second.id << " " 
		//		<< itr->second.pos.x << " " << itr->second.pos.y << " " << itr->second.pos.z << " " 
		//		<< itr->second.color.x << " " << itr->second.color.y << " " << itr->second.color.z << " " 
		//		<< itr->second.error << " ";
		//	for (auto &item : itr->second.idImgs) {
		//		ofs << item.first << " " << item.second << " ";
		//	}
		//	ofs << std::endl;
		//}
		
		ofs.close();
	}

	std::ofstream ofs(sPath + "/colmap_text_01/images.txt");

	for (auto itr = images1.begin(); itr != images1.end(); itr++) {
		auto itrPt = images0.find(itr->second.id);
		if (itrPt != images0.end()) {
			itrPt->second.points2d.insert(itr->second.points2d.begin(), itr->second.points2d.end());
			continue;
		}

		itr->second.trans.postMult(combMat);
		osg::Quat q = itr->second.trans.getRotate();
		osg::Vec3 t = itr->second.trans.getTrans();
		transRt(q, t);
		itr->second.q = q;
		itr->second.t = t;

		ofs << itr->second.id << " " << itr->second.q.w() << " " << itr->second.q.x() << " " << itr->second.q.y() << " " << itr->second.q.z() << " " << itr->second.t.x() << " " << itr->second.t.y() << " " << itr->second.t.z() << " " << itr->second.idCamera << " " << itr->second.name << std::endl;

		bool notbegin = false;
		for (auto itrPt = itr->second.points2d.begin(); itrPt != itr->second.points2d.end(); itrPt++) {
			if (notbegin) ofs << " ";
			ofs << itrPt->second.x << " " << itrPt->second.y << " " << itrPt->first;
			notbegin = true;
		}
		ofs << std::endl;
	}

	//for (auto itr = images0.begin(); itr != images0.end(); itr++) {
	//	ofs << itr->second.id << " " << itr->second.q.w() << " " << itr->second.q.x() << " " << itr->second.q.y() << " " << itr->second.q.z() << " " << itr->second.t.x() << " " << itr->second.t.y() << " " << itr->second.t.z() << " " << itr->second.idCamera << " " << itr->second.name << std::endl;
	//	bool notbegin = false;
	//	for (auto itrPt = itr->second.points2d.begin(); itrPt != itr->second.points2d.end(); itrPt++) {
	//		if (notbegin) ofs << " ";
	//		ofs << itrPt->second.x << " " << itrPt->second.y << " " << itrPt->first;
	//		notbegin = true;
	//	}
	//	ofs << std::endl;
	//}

	ofs.close();
}

void OsgManager::showFrame() {
	std::string sImageFile = "E:/projects/instant-ngp/data/nerf/lab_sub/colmap_text/images.txt";

	std::unordered_map<int, Point3D> points3d = loadPoint3d();
	std::unordered_map<int, ImageSt> images = loadImages();
	//std::cout << "load params success" << std::endl;
	{
		std::ifstream ifs(sImageFile);
		std::string line;
		int lineIdx = 0;
		while (!ifs.eof()) {
			std::getline(ifs, line);
			if (line.length() <= 0) continue;
			if (line[0] == '#') continue;

			std::vector<Vector2> vProjPoints2d;
			osg::ref_ptr<osg::Geometry> pTargetGeom = new osg::Geometry;
			osg::ref_ptr<osg::Vec3Array> vTargetVertex = new osg::Vec3Array;
			osg::ref_ptr<osg::Vec3Array> vTargetNormal = new osg::Vec3Array;
			osg::ref_ptr<osg::Vec2Array> vTargetUv = new osg::Vec2Array;
			std::vector<unsigned int> vTargetIndices;
			Vector3 vVecLocation;

			if ((lineIdx & 1) == 0) {

				std::vector<std::string> params = splitString(line, ' ');
				if (params.size() < 9) {
					std::cout << "error " << std::endl;
					continue;
				}

				//std::cout << "load frame " << (lineIdx >> 1) << std::endl;

				int idImg = std::stoi(params[0]);

				osg::Vec3 vLocation = images[idImg].trans.getTrans();
				vVecLocation = Vector3(vLocation.x(), vLocation.y(), vLocation.z());
				Vector3 vProjNormal(1);
				vProjNormal.normalize();
				Vector3 vProjPosition = vVecLocation;
				CommonPlane projPlane(vProjNormal, vProjPosition);


				std::getline(ifs, line);

				std::vector<std::string> points2d = splitString(line, ' ');
				for (int i = 2; i < points2d.size(); i += 3) {
					int idPoint3d = std::stoi(points2d[i]);
					if (idPoint3d >= 0) {
						Point2D featurePt;
						featurePt.x = std::stof(points2d[i - 2]);
						featurePt.y = std::stof(points2d[i - 1]);


						vProjPoints2d.push_back(Vector2(featurePt.x, featurePt.y));

						vTargetVertex->push_back(osg::Vec3(points3d[idPoint3d].pos.x, points3d[idPoint3d].pos.y, points3d[idPoint3d].pos.z));
						vTargetNormal->push_back(osg::Vec3(0, 0, 0));
						vTargetUv->push_back(osg::Vec2(featurePt.x / 4032, featurePt.y / 3024));
					}
				}

				//std::cout << "frame id " << idImg << std::endl;

				Graph_Geometry::Delaunay delaunay;
				Graph_Geometry::GraphGeometry T = delaunay.triangulate(vProjPoints2d);

				//std::cout << "output face " << T.faces.size() << std::endl;

				for (int i = 0; i < T.faces.size(); ++i) {
					HalfEdge h = T.outerComponent(T.faces[i]);
					int p0 = T.origin(h).id;
					int pi = T.origin(T.next(h)).id;
					int pj = T.origin(T.prev(h)).id;

					if (p0 < 0 || pi < 0 || pj < 0) continue;
					int index0 = delaunay.m_mapIn2outIndex[p0];
					int index1 = delaunay.m_mapIn2outIndex[pi];
					int index2 = delaunay.m_mapIn2outIndex[pj];

					Triangle triangle = Triangle(Vector3(vTargetVertex->at(index0).x(), vTargetVertex->at(index0).y(), vTargetVertex->at(index0).z()),
						Vector3(vTargetVertex->at(index1).x(), vTargetVertex->at(index1).y(), vTargetVertex->at(index1).z()),
						Vector3(vTargetVertex->at(index2).x(), vTargetVertex->at(index2).y(), vTargetVertex->at(index2).z()));
					Vector3 vFaceNormal = triangle.getNormal();

					{
						float threshold = std::cos(osg::PI / 6.f);
						Vector3 vProjLine = (triangle.v0 - vVecLocation).getNormalize();

						Vector3 edge = (triangle.v0 - triangle.v1).getNormalize();
						if (std::fabs(vProjLine.dot(edge)) > threshold) continue;
						edge = (triangle.v0 - triangle.v2).getNormalize();
						if (std::fabs(vProjLine.dot(edge)) > threshold) continue;

						vProjLine = (triangle.v1 - vVecLocation).getNormalize();

						edge = (triangle.v1 - triangle.v2).getNormalize();
						if (std::fabs(vProjLine.dot(edge)) > threshold) continue;
						edge = (triangle.v1 - triangle.v0).getNormalize();
						if (std::fabs(vProjLine.dot(edge)) > threshold) continue;

						vProjLine = (triangle.v2 - vVecLocation).getNormalize();

						edge = (triangle.v2 - triangle.v1).getNormalize();
						if (std::fabs(vProjLine.dot(edge)) > threshold) continue;
						edge = (triangle.v2 - triangle.v0).getNormalize();
						if (std::fabs(vProjLine.dot(edge)) > threshold) continue;
					}
					float threshold = 2.f;
					if (triangle.v0.distanceTo(triangle.v1) > threshold) continue;
					if (triangle.v0.distanceTo(triangle.v2) > threshold) continue;
					if (triangle.v2.distanceTo(triangle.v1) > threshold) continue;

					vTargetIndices.push_back(index0);
					vTargetIndices.push_back(index1);
					vTargetIndices.push_back(index2);

					vTargetNormal->at(index0) += osg::Vec3(vFaceNormal.x, vFaceNormal.y, vFaceNormal.z);
					vTargetNormal->at(index1) += osg::Vec3(vFaceNormal.x, vFaceNormal.y, vFaceNormal.z);
					vTargetNormal->at(index2) += osg::Vec3(vFaceNormal.x, vFaceNormal.y, vFaceNormal.z);
				}

				pTargetGeom->setVertexArray(vTargetVertex);
				pTargetGeom->setNormalArray(vTargetNormal, osg::Array::Binding::BIND_PER_VERTEX);
				pTargetGeom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, vTargetIndices.size(), vTargetIndices.data()));


				//shader(pTargetGeom, images[idImg].name);
				pTargetGeom->setVertexAttribArray(1, vTargetVertex, osg::Array::Binding::BIND_PER_VERTEX);
				pTargetGeom->setVertexAttribArray(2, vTargetUv, osg::Array::Binding::BIND_PER_VERTEX);

				root->addChild(pTargetGeom);
			}
			lineIdx += 2;

			break;
		}
	}
}

