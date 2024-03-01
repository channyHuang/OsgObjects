#include "osgManager.h"

#include "osgPickHandler.h"

#include "commonMath/vector2.h"
#include "commonMath/vector3.h"
#include "commonMath/plane.h"
#include "commonMath/funcs.h"
#include "commonMath/triangle.h"
#include "commonGeometry/delaunay.h"

#include "commonOsg/commonOsg.h"

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

std::vector<osg::Matrix> getPose(const std::string& sPoseName) {
	std::vector<osg::Matrix> trans;
	std::ifstream ifs(sPoseName);
	while (!ifs.fail() && !ifs.eof()) {
		std::string line;
		std::getline(ifs, line);
		if (line.length() < 8) continue;
		std::vector<std::string> res = splitString(line, ' ');

		osg::Matrix tr = osg::Matrix::rotate(osg::Quat(std::atof(res[4].c_str()), std::atof(res[5].c_str()), std::atof(res[6].c_str()), std::atof(res[7].c_str())));
		tr = tr.translate(osg::Vec3(std::atof(res[1].c_str()), std::atof(res[2].c_str()), std::atof(res[3].c_str())));

		trans.push_back(tr);
	}
	return trans;
}

bool sign(float v) { return (v > 0); }
bool samesign(float a, float b) { return ((a >= 0) ? (b >= 0) : (b < 0)); }
bool samearea(Vector3 a, Vector3 b) {
	if (!samesign(a.x, b.x)) return false;
	if (!samesign(a.y, b.y)) return false;
	if (!samesign(a.z, b.z)) return false;
	return true;
}

osg::Matrix getDatasetPose(const std::string& sPoseName) {

	std::ifstream ifs(sPoseName);
	std::string line;
	std::getline(ifs, line);
	std::vector<std::string> pos = splitString(line, ' ');
	std::getline(ifs, line);
	std::vector<std::string> rot = splitString(line, ' ');

	osg::Matrix mat;
	mat.makeIdentity();
	osg::Matrix tr = mat * osg::Matrix::rotate(osg::DegreesToRadians(std::atof(rot[0].c_str())), osg::Vec3(1.0f, 0.0f, 0.0f),
		osg::DegreesToRadians(std::atof(rot[1].c_str())), osg::Vec3(0.0f, 1.0f, 0.0f),
		osg::DegreesToRadians(std::atof(rot[2].c_str())), osg::Vec3(0.0f, 0.0f, 1.0f));

	//osg::Matrix tr = mat * osg::Matrix::rotate(std::atof(rot[0].c_str()), osg::Vec3(1.0f, 0.0f, 0.0f),
	//	std::atof(rot[1].c_str()), osg::Vec3(0.0f, 1.0f, 0.0f),
	//	std::atof(rot[2].c_str()), osg::Vec3(0.0f, 0.0f, 1.0f));

	tr.translate(osg::Vec3(std::atof(pos[0].c_str()), std::atof(pos[1].c_str()), std::atof(pos[2].c_str())));

	return tr;
}

void OsgManager::showDataset() {
	std::string sFilePath = "D:/dataset/thermocolorlab"; // only for test 
	int stpos = 0, endpos = 9;
	char cIndex[10] = { 0 };
	float degree = 20.f; // 40 degree rotation between frames

	if (m_nDatasetPos >= endpos) {
		m_nDatasetPos = stpos;
	}

	sprintf(cIndex, "%03d", m_nDatasetPos);

	osg::ref_ptr<osg::Geometry> pGeom = createGeomUsingTinyobj((sFilePath + "/scan" + std::string(cIndex) + ".obj").c_str(), "");
	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

	std::string sPoseName = sFilePath + "/scan" + std::string(cIndex) + ".pose";
	osg::Matrix matTranslate = getDatasetPose(sPoseName);

	Vector3 vProjNormal(-std::sin(degree * MathFuncs::PI / 180.f), 0, std::cos(degree * MathFuncs::PI / 180.f));
	vProjNormal.normalize();
	Vector3 vProjPosition(vProjNormal.x * 100.f / vProjNormal.z, 0.f, 100.f);
	Plane projPlane(vProjNormal, vProjPosition);

	std::vector<Vector2> vProjPoints2d;

	osg::ref_ptr<osg::Geometry> pTargetGeom = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> vTargetVertex = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> vTargetNormal = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> vTargetColor = (osg::Vec4Array*)pGeom->getColorArray();
	std::vector<unsigned int> vTargetIndices;

	//std::ofstream ofs("E:/dataset/thermocolorlab_res/scan" + std::string(cIndex) + "_res.obj");
	for (int i = 0; i < varray->size(); ++i) {
		Vector3 vOriginPoint = Vector3(varray->at(i).x(), varray->at(i).y(), varray->at(i).z());
		Vector3 vProjPoint = MathFuncs::linePlaneIntersect(vOriginPoint, Vector3(0), projPlane);

		vProjPoints2d.push_back(Vector2(vProjPoint.x, vProjPoint.y));

		osg::Vec3 vRotPoint = matTranslate.preMult(varray->at(i));
		vTargetVertex->push_back(vRotPoint);
		vTargetNormal->push_back(osg::Vec3(0, 0, 0));
	}

	Graph_Geometry::Delaunay delaunay;
	Graph_Geometry::GraphGeometry T = delaunay.triangulate(vProjPoints2d);

	for (int i = 0; i < T.faces.size(); ++i) {
		HalfEdge h = T.outerComponent(T.faces[i]);
		int p0 = T.origin(h).id;
		int pi = T.origin(T.next(h)).id;
		int pj = T.origin(T.prev(h)).id;

		if (p0 < 0 || pi < 0 || pj < 0) continue;
		int index0 = delaunay.m_mapIn2outIndex[p0];
		int index1 = delaunay.m_mapIn2outIndex[pi];
		int index2 = delaunay.m_mapIn2outIndex[pj];

		Triangle triangle = Triangle(Vector3(varray->at(index0).x(), varray->at(index0).y(), varray->at(index0).z()),
			Vector3(varray->at(index1).x(), varray->at(index1).y(), varray->at(index1).z()),
			Vector3(varray->at(index2).x(), varray->at(index2).y(), varray->at(index2).z()));
		Vector3 vFaceNormal = triangle.getNormal();
		//if ((Vector3(0) - dir).dot(face_normal) < std::cos(Math::PI / 3.f)) continue;
		if (triangle.v0.distanceTo(triangle.v1) > 10.f) continue;
		if (triangle.v0.distanceTo(triangle.v2) > 10.f) continue;
		if (triangle.v2.distanceTo(triangle.v1) > 10.f) continue;

		vTargetIndices.push_back(index0);
		vTargetIndices.push_back(index1);
		vTargetIndices.push_back(index2);

		vTargetNormal->at(index0) += osg::Vec3(vFaceNormal.x, vFaceNormal.y, vFaceNormal.z);
		vTargetNormal->at(index1) += osg::Vec3(vFaceNormal.x, vFaceNormal.y, vFaceNormal.z);
		vTargetNormal->at(index2) += osg::Vec3(vFaceNormal.x, vFaceNormal.y, vFaceNormal.z);

		//ofs << "f " << index0 + 1 << " " << index1 + 1 << " " << index2 + 1 << std::endl;
	}
	//ofs.close();

	pTargetGeom->setVertexArray(vTargetVertex);
	pTargetGeom->setNormalArray(vTargetNormal, osg::Array::Binding::BIND_PER_VERTEX);
	pTargetGeom->setColorArray(vTargetColor, osg::Array::Binding::BIND_PER_VERTEX);
	pTargetGeom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, vTargetIndices.size(), vTargetIndices.data()));

	sceneSwitch->addChild(pTargetGeom);

	m_nDatasetPos++;
}

void OsgManager::show(const std::string& sFilePath) {
	std::string sPoseName = sFilePath + "\\optimized_pose.txt";
	std::vector<osg::Matrix> vMatTranslate = getPose(sPoseName);

	int stpos = 334, endpos = 430;
	char cIndex[10] = { 0 };

	std::vector<Vector3> vPlaneNormals = { Vector3(1, 1, 1), Vector3(1, 1, -1), Vector3(1, -1, 1), Vector3(-1, 1, 1),
											Vector3(-1, -1, -1), Vector3(1, -1, -1), Vector3(-1, 1, -1), Vector3(-1, -1, 1) };

	for (int idx = stpos; idx < endpos; ++idx) {
		if (idx == 9 || idx == 64 || idx == 115 || idx == 130 || idx == 167 || idx == 333) continue;
		sprintf(cIndex, "%d", idx);
		osg::Vec3 vLocation = vMatTranslate[idx].getTrans();
		Vector3 vVecLocation = Vector3(vLocation.x(), vLocation.y(), vLocation.z());
		osg::ref_ptr<osg::Geometry> pGeom = createGeomUsingTinyobj((sFilePath + "/" + std::string(cIndex) + ".obj").c_str(), "");

		std::size_t pos = sFilePath.find_last_of('.');

		osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

		std::ofstream ofs("E:\\tmp\\" + std::string(cIndex) + "_res.obj");
		int offset = 0;
		for (int j = 0; j < 8; ++j) {
			Vector3 vProjNormal = vPlaneNormals[j];
			vProjNormal.normalize();
			Vector3 vProjPosition = vProjNormal * 0.1f;
			Plane projPlane(vProjNormal, vProjPosition);

			std::vector<Vector2> vProjPoints2d;

			osg::ref_ptr<osg::Geometry> pTargetGeom = new osg::Geometry;
			osg::ref_ptr<osg::Vec3Array> vTargetVertex = new osg::Vec3Array;
			osg::ref_ptr<osg::Vec3Array> vTargetNormal = new osg::Vec3Array;
			std::vector<unsigned int> vTargetIndices;

			for (int i = 0; i < varray->size(); ++i) {
				Vector3 vOriginPoint = Vector3(varray->at(i).x(), varray->at(i).y(), varray->at(i).z());
				if (!samearea(vOriginPoint, vPlaneNormals[j])) continue;

				Vector3 vProjPoint = MathFuncs::linePlaneIntersect(vOriginPoint, Vector3(0), projPlane);

				vProjPoints2d.push_back(Vector2(vProjPoint.x * sqrt(2), vProjPoint.y * sqrt(2)));

				osg::Vec3 vRotPoint = vMatTranslate[idx].preMult(varray->at(i));
				vTargetVertex->push_back(vRotPoint);
				vTargetNormal->push_back(osg::Vec3(0, 0, 0));

				ofs << "v " << vRotPoint.x() << " " << vRotPoint.y() << " " << vRotPoint.z() << std::endl;
			}

			Graph_Geometry::Delaunay delaunay;
			Graph_Geometry::GraphGeometry T = delaunay.triangulate(vProjPoints2d);

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

				ofs << "f " << index0 + 1 + offset << " " << index1 + 1 + offset << " " << index2 + 1 + offset << std::endl;
			}

			pTargetGeom->setVertexArray(vTargetVertex);
			pTargetGeom->setNormalArray(vTargetNormal, osg::Array::Binding::BIND_PER_VERTEX);
			pTargetGeom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, vTargetIndices.size(), vTargetIndices.data()));
			root->addChild(pTargetGeom);

			offset += vTargetVertex->size();
		}
		ofs.close();
	}

}
