#include "osgManager.h"

#include "commonOsg/commonOsg.h"
#include <filesystem>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

OsgManager* OsgManager::instance = nullptr;

OsgManager::OsgManager() {
	root = new osg::Group;
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	root->addChild(createAxis());

	sceneSwitch = new osg::Switch;
	sceneSwitch->setAllChildrenOn();
	root->addChild(sceneSwitch);

	rootGeomDistance = new osg::Group;
	sceneSwitch->addChild(rootGeomDistance);
}

OsgManager::~OsgManager() {
	pviewer.release();
}

void OsgManager::setViewer(osgViewer::Viewer& viewer) {
	pviewer = &viewer;

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

void OsgManager::showPick(osg::Vec3& p) {
	m_vPickPoints.push_back(p);

	osg::ref_ptr<osg::Geometry> geom = createPlanet(.02f, osg::Vec4(1, 0, 0, 0), p);
	rootGeomDistance->addChild(geom);
	if ((m_vPickPoints.size() & 1) == 0) {
		float dist = (m_vPickPoints[m_vPickPoints.size() - 1] - m_vPickPoints[m_vPickPoints.size() - 2]).length() * 1.83 ; // 0.44 / 0.24
		
		osg::ref_ptr<osg::Geometry> lgeom = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> vertex = new osg::Vec3Array();
		vertex->push_back(m_vPickPoints[m_vPickPoints.size() - 1]);
		vertex->push_back(m_vPickPoints[m_vPickPoints.size() - 2]);
		lgeom->setVertexArray(vertex);
		lgeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, vertex->size()));

		osg::Vec3 pos = (m_vPickPoints[m_vPickPoints.size() - 1] + m_vPickPoints[m_vPickPoints.size() - 2]) / 2;

		rootGeomDistance->addChild(lgeom);
		rootGeomDistance->addChild(createText(pos, 25, std::to_string(dist) + "m"));
	}
}

void OsgManager::clearPick() {
	m_vPickPoints.clear();
	rootGeomDistance->removeChildren(0, rootGeomDistance->getNumChildren());
}

void OsgManager::showModelObj(const std::string &str, const std::string& sTex, bool bUseOsgLoad) {
	if (!bUseOsgLoad) {
		loadGeomUsingTinyobj(str.c_str(), sTex.c_str(), pviewer);
	}
	else {
		osg::ref_ptr<osg::Geometry> geom = createGlslGeomUsingTinyobj(str.c_str(), sTex.c_str(), pviewer); //createGeomUsingTinyobj(str.c_str(), "");
		sceneSwitch->addChild(geom);

		osg::ref_ptr<osg::Geometry> geomWireframe = new osg::Geometry(*geom.get(), osg::CopyOp::SHALLOW_COPY);
		osg::ref_ptr<osg::Vec4Array> vcolors = new osg::Vec4Array;
		vcolors->push_back(osg::Vec4(0.f, 1.f, 0.f, 1.f));
		geomWireframe->setColorArray(vcolors, osg::Array::Binding::BIND_OVERALL);
		setWireFrame(geomWireframe->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);
		sceneSwitch->addChild(geomWireframe);
	}
}

void OsgManager::obj2osgt(const std::string& str) {
	osg::ref_ptr<osg::Geometry> geom = createGeomUsingTinyobj(str.c_str(), "");
	osgDB::writeNodeFile(*geom, std::string("./hud.osgt"));
}

void OsgManager::showModelOsgbLOD(const std::string& str, const std::string& sTex) {
	auto pos = str.find_last_of('\\');
	std::string path = str.substr(0, pos);

	osg::ref_ptr<osg::LOD> pLod = new osg::LOD;
	pLod->addChild(osgDB::readNodeFile(path + "/Model.osgb"), 0, 20);
	sceneSwitch->addChild(pLod);
}

osg::ref_ptr<osg::Geometry> loadModelFileByAssimp(const std::string& sFileName, const std::string& sTextrueName) {
	const aiScene* scene = aiImportFile(sFileName.c_str(), aiProcess_Triangulate);
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	if (scene) {
		for (int x = 0; x < scene->mNumMeshes; ++x) {
			aiMesh* mesh = scene->mMeshes[x];
			if (mesh->mNumVertices <= 0) continue;

			osg::ref_ptr<osg::Vec3Array> vvertex = new osg::Vec3Array(mesh->mNumVertices);
			osg::ref_ptr<osg::Vec2Array> vcoords = new osg::Vec2Array(mesh->mNumVertices);
			osg::ref_ptr<osg::Vec3Array> vnormal = new osg::Vec3Array(mesh->mNumVertices);
			osg::ref_ptr<osg::Vec4Array> vcolors = new osg::Vec4Array(mesh->mNumVertices);

#pragma omp parallel for num_threads(30)
			for (int i = 0; i < mesh->mNumVertices; ++i) {
				vvertex->at(i) = (osg::Vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
				if (mesh->HasNormals()) {
					vnormal->at(i) = (osg::Vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
				}
				if (mesh->HasTextureCoords(0)) {
					aiVector3D* coords = mesh->mTextureCoords[0];
					vcoords->at(i) = (osg::Vec2(coords[i].x, coords[i].y));
				}
				if (mesh->HasVertexColors(0)) {
					vcolors->at(i) = (osg::Vec4(mesh->mColors[i]->r, mesh->mColors[i]->g, mesh->mColors[i]->b, mesh->mColors[i]->a));
				}
			}

			if (mesh->mNumFaces) {
				std::vector<unsigned int> vindices;
				for (int i = 0; i < mesh->mNumFaces; ++i) {
					for (int j = 0; j < mesh->mFaces[i].mNumIndices; ++j) {
						vindices.push_back(mesh->mFaces[i].mIndices[j]);
					}
				}
				geom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, vindices.size(), vindices.data()));
			}
			else {
				geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, vvertex->size()));
			}

			geom->setVertexArray(vvertex.get());
			if (mesh->HasNormals()) {
				geom->setNormalArray(vnormal.get());
			}
			if (mesh->HasTextureCoords(0)) {
				geom->setTexCoordArray(0, vcoords.get());
			}
		}
	}
	return geom;
}

void OsgManager::showModelUsingAssimp(std::string sFileName, std::string sTextureName) {
	if (sFileName.length() == 0) {
		sFileName = "E:/thirdLibs/cgal-master/Data/data/meshes/3torus.off";
	}
	osg::ref_ptr<osg::Geometry> geom = loadModelFileByAssimp(sFileName, sTextureName);

	osg::ref_ptr<osg::Geometry> geomWireframe = new osg::Geometry(*geom.get(), osg::CopyOp::DEEP_COPY_ALL);
	osg::ref_ptr<osg::Vec4Array> vcolors = new osg::Vec4Array;
	vcolors->push_back(osg::Vec4(0.f, 1.f, 0.f, 1.f));
	geomWireframe->setColorArray(vcolors, osg::Array::Binding::BIND_OVERALL);
	setWireFrame(geomWireframe->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);
	sceneSwitch->addChild(geomWireframe);

	if (sTextureName.length() > 0) {
		createOsgGlslNode(geom, pviewer, sTextureName);
	}
	sceneSwitch->addChild(geom);
}
