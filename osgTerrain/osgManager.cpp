#include "osgManager.h"

#include "osgPickHandler.h"
//#include "tiny_obj_loader.h"

OsgManager* OsgManager::instance = nullptr;

OsgManager::OsgManager() {
	root = new osg::Group;
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

	rootWireTerrain = new osg::Group;
	rootWireTerrain->addChild(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.1f)));
	rootGeomTerrain = new osg::Group;//createLight(osg::Vec3(0.f, 0.f, 5.f), 1, bs, root);
	rootGeomTerrain->addChild(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.1f)));

	sceneSwitch = new osg::Switch;
	sceneSwitch->addChild(rootGeomTerrain);
	sceneSwitch->addChild(rootWireTerrain);
	sceneSwitch->setAllChildrenOn();
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
	
	//sunLight = createSunLight();
	//root->addChild(sunLight);
	root->addChild(createSkyBox());

	//sunLight->addChild(createAxis());
	//sunLight->addChild(sceneSwitch);
	root->addChild(createAxis());
	root->addChild(sceneSwitch);

	bs = osg::BoundingSphere(osg::Vec3(0.f, 0.f, 0.f), 1.f);

	brush = new VoxelBrush(TerrainManager::getInstance(), VoxelMap::getInstance());

//	precon = Reconstruct::getInstance();

	updateShow();
}

void OsgManager::clear() {
	if (rootGeomTerrain.valid()) {
		if (rootGeomTerrain->getNumChildren()) {
			rootGeomTerrain->removeChildren(1, rootGeomTerrain->getNumChildren());
		}
	} 
	if (rootWireTerrain.valid()) {
		if (rootWireTerrain->getNumChildren()) {
			rootWireTerrain->removeChildren(1, rootWireTerrain->getNumChildren());
		}
	}
	sceneMap.clear();
	sceneSwitch->removeChildren(2, sceneSwitch->getNumChildren());
	bs = osg::BoundingSphere(osg::Vec3(0.f, 0.f, 0.f), 1.f);

	updateShow();
}

osg::ref_ptr<osg::Geometry> OsgManager::getMeshGeometry(const Arrays& surface, osg::Vec3 color) {
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	for (auto& pt : surface.positions) {
		vertices->push_back(osg::Vec3(pt.x, pt.y, pt.z));
	}
	geom->setVertexArray(vertices.get());

	GLuint* indices = new GLuint[surface.indices.size()];
	int i = 0;
	for (auto& vidx : surface.indices) {
		indices[i++] = vidx;
	}

	osg::Vec3Array* normals = new osg::Vec3Array;
	for (auto& n : surface.normals) {
		normals->push_back(osg::Vec3(n.x, n.y, n.z));
	}
	geom->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;
	for (auto& uv : surface.uvs) {
		tcoords->push_back(osg::Vec2(uv.x, uv.y));
	}
	geom->setTexCoordArray(0, tcoords.get(), osg::Array::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(color.x(), color.y(), color.z(), 1.f));
	geom->setColorArray(colors.get(), osg::Array::BIND_OVERALL);

	geom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, surface.indices.size(), indices));
	return geom;
}

osg::ref_ptr<osgShadow::ShadowedScene> OsgManager::getShadowScene(osg::ref_ptr<osg::Geometry> geomNode, ShowType type) {
	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;

	osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
	settings->setReceivesShadowTraversalMask(0x01);
	settings->setCastsShadowTraversalMask(0x02);

	osg::StateSet* stateset = geomNode->getOrCreateStateSet();
	setWireFrame(stateset, type);
	setTexture("texture/basecolor.bmp", stateset);

	osg::ref_ptr<osg::Material> matirial = createMaterial();
	stateset->setAttributeAndModes(matirial.get(), osg::StateAttribute::ON);
	
	shadowedScene->addChild(geomNode);

	return shadowedScene;
}

osg::ref_ptr<osg::Geometry> OsgManager::getModelGeometry(const char* pName, bool bPointsOnly) {
	osg::ref_ptr<osg::Geometry> geom = loadModelFile(pName);
	const clock_t startTime = clock();
	/*
	bool res = precon->loadMesh(pName);

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::Vec3Array* normals = new osg::Vec3Array;
 
	for (auto& pt : precon->vcgMesh.vert) {
		vertices->push_back(osg::Vec3(pt.P().X(), pt.P().Y(), pt.P().Z()));

		if (!bPointsOnly) {
			normals->push_back(osg::Vec3(pt.N().X(), pt.N().Y(), pt.N().Z()));
		}
	}
	geom->setVertexArray(vertices.get());

	if (bPointsOnly) {
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, vertices.get()->size()));
		return geom;
	}

	geom->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);

	GLuint* indices = new GLuint[precon->vcgMesh.FN() * 3];
	int i = 0;
	for (MyMesh::FaceIterator fi = precon->vcgMesh.face.begin(); fi != precon->vcgMesh.face.end(); ++fi) {
		
		indices[i++] = tri::Index(precon->vcgMesh, fi->V(0));
		indices[i++] = tri::Index(precon->vcgMesh, fi->V(1));
		indices[i++] = tri::Index(precon->vcgMesh, fi->V(2));
	}
	*/
	osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;
	std::vector<std::pair<float, float>> uvs;
	//MeshPostProcessing::getInstance()->unwrap(pName, uvs);
	for (auto& uv : uvs) {
		//tcoords->push_back(osg::Vec2(uv.first, uv.second));
		tcoords->push_back(osg::Vec2(rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX));
	}
	geom->setTexCoordArray(0, tcoords.get(), osg::Array::BIND_PER_VERTEX);

	//geom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, precon->vcgMesh.FN() * 3, indices));

	const double elapsedTime = (clock() - startTime) * 1000.0 / CLOCKS_PER_SEC;
	std::cout << "spand time " << elapsedTime / 1000.f << " seconds" << std::endl;

	return geom;
}

osg::ref_ptr<osg::Node> OsgManager::getModelNode(const char* pName, bool bPointsOnly, osg::Vec4 color) {
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	osg::ref_ptr<osg::Geometry> geom = getModelGeometry(pName, bPointsOnly);
	if (!bPointsOnly) {
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
		colors->push_back(color);
		geom->setColorArray(colors.get(), osg::Array::BIND_OVERALL);
	}
	
	geode->addDrawable(geom);
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	return geode;
}

void OsgManager::setNormalMap(osg::ref_ptr<osg::Node> geomNode, osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene) {
	osg::ref_ptr<osg::Program> program = loadShaderPrograms("normalmap", "glsl/vshader_osg.glsl", "glsl/fshader_osg.glsl");
	osg::ref_ptr<osg::StateSet> ss = geomNode->getOrCreateStateSet();
	ss->setAttributeAndModes(program.get(), osg::StateAttribute::ON);

	ss->setTextureAttributeAndModes(0, createTexture("texture/basecolor.bmp"));
	ss->setTextureAttributeAndModes(1, createTexture("texture/normal.bmp"));
	ss->addUniform(new osg::Uniform("diffMap", 0));
	ss->addUniform(new osg::Uniform("bumpMap", 1));
	ss->addUniform(new osg::Uniform("useBumpMap", 1));

	float modelSize = bs.radius();
	osg::Vec3 corner = bs.center() + osg::Vec3(modelSize, modelSize, modelSize) * 0.5f;
	ss->addUniform(new osg::Uniform("lightPos", corner));
}

void OsgManager::showModelScene(const char* pName) {
	//clear();
	if (1) {
		osg::ref_ptr<osg::Node> geomNode = getModelNode(pName, false, osg::Vec4(0.5f, 0.5f, 0.5f, 1.f));
		sceneSwitch->addChild(geomNode);
		bs.expandBy(geomNode->getBound());
		osg::StateSet* stateset = geomNode->getOrCreateStateSet();

		osg::ref_ptr<osg::Material> matirial = createMaterial();
		stateset->setAttributeAndModes(matirial.get(), osg::StateAttribute::ON);
		setWireFrame(stateset, ShowType::SHOW_FRONT_AND_BACK);
		setTexture("texture/basecolor.bmp", stateset);
		setTexCombine(stateset);
	}
	if (1) {
		osg::ref_ptr<osg::Node> wireNode = getModelNode(pName, false, osg::Vec4(0.f, 1.f, 0.f, 1.f));
		bs = wireNode->getBound();
		setWireFrame(wireNode->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);
		sceneSwitch->addChild(wireNode);
	}
	if (0) {
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		settings->setReceivesShadowTraversalMask(0x01);
		settings->setCastsShadowTraversalMask(0x02);

		osg::ref_ptr<osg::Node> geomNode = getModelNode(pName, false, osg::Vec4(.5f, .5f, .5f, 1.f));
		bs.expandBy(geomNode->getBound());
		setWireFrame(geomNode->getOrCreateStateSet(), ShowType::SHOW_FRONT);
		
		osg::StateSet* stateset = geomNode->getOrCreateStateSet();
		setTexture("texture/basecolor.bmp", stateset);

		shadowedScene->addChild(geomNode);
		shadowedScene->addChild(createLight(osg::Vec3(0.f, 0.f, 1.f), 1, bs, root));

		osg::ref_ptr<osg::LightSource> light = createLight(osg::Vec3(0.f, 0.f, 0.f), 1, bs, root);
		light->addChild(shadowedScene);
		sceneSwitch->addChild(light);
	}
}

void OsgManager::updateTerrain(Arrays surface, Vector3i pos) {
	osg::ref_ptr<osg::Geometry> geom = getMeshGeometry(surface);
	bs.expandBy(geom->getBound());
	
	osg::StateSet* stateset = geom->getOrCreateStateSet();
	osg::ref_ptr<osg::Material> matirial = createMaterial();
	stateset->setAttributeAndModes(matirial.get(), osg::StateAttribute::ON);
	setWireFrame(stateset, ShowType::SHOW_FRONT_AND_BACK);
	setTexture("texture/basecolor.bmp", stateset);
	setTexCombine(stateset);

	osg::ref_ptr<osg::Geometry> geomFrame = getMeshGeometry(surface, osg::Vec3(0.f, 1.f, 0.f));
	setWireFrame(geomFrame->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);

	std::unique_lock<std::mutex> locker(mutex);
	std::unordered_map < Vector3i, std::pair<osg::ref_ptr<osg::Geometry>, osg::ref_ptr<osg::Geometry>>>::iterator scene = sceneMap.find(pos);
	
	if (scene != sceneMap.end()) {
		rootGeomTerrain->replaceChild(scene->second.first, geom);
		rootWireTerrain->replaceChild(scene->second.second, geomFrame);
	}
	else {
		rootGeomTerrain->addChild(geom);
		rootWireTerrain->addChild(geomFrame);
	}
	sceneMap.insert(std::make_pair(pos, std::make_pair(geom, geomFrame)));

	
	locker.unlock();

#ifdef PHYSICS_ON
	PhysicsManager::getInstance()->addTerrain(pos, surface);
#endif
}

void OsgManager::updateShow() {
	if (!root.valid()) {
		std::cerr << "-------- invalid " << std::endl;
		return;
	}
	
	if (pviewer != nullptr) {
		pviewer->setSceneData(root);
	}
}

void OsgManager::test() {
	loadObjectModel("D:/osgfile/obj/textured_output.obj", "D:/osgfile/obj/", pviewer.get());
}

std::string OsgManager::createSpheres(const osg::Vec3& pos) {
	osg::ref_ptr<osg::Geometry> geom = createPlanet(1.f);//new osg::ShapeDrawable(new osg::Sphere(pos, 1.f));
	std::string id = Common::genUUID();
	osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
	osg::Matrix projMatrix;
	projMatrix.makeIdentity();
	projMatrix.setTrans(pos);
	transform->setMatrix(projMatrix);
	transform->addChild(geom);
#ifdef PHYSICS_ON
	if (!physicsRoot.valid()) {
		physicsRoot = new osg::Group;
		sceneSwitch->addChild(physicsRoot);
	}

	physicsSphere[id] = std::make_pair(geom, transform);
	physicsRoot->addChild(transform);
	qu.push(id);

	if (physicsSphere.size() >= 20) {
		std::string oldid = qu.front();
		removeDynamicActors(oldid);
		qu.pop();
	}

	PhysicsManager::getInstance()->createDynamic(id, physx::PxTransform(physx::PxVec3(pos.x(), pos.y(), pos.z())));
#endif
	//std::cout << __FUNCTION__ << " " << id.c_str() << " " << pos.x() << " " << pos.y() << " " << pos.z() << std::endl;

	updateShow();
	return id;
}

void OsgManager::removeDynamicActors(const std::string& id) {
#ifdef PHYSICS_ON
	PhysicsManager::getInstance()->removeDynamic(id);
#endif
	auto itr = physicsSphere.find(id);
	if (itr != physicsSphere.end()) {
		itr->second.second->removeChild(itr->second.first);
		physicsRoot->removeChild(itr->second.second);

		physicsSphere.erase(id);
	}
}
#ifdef PHYSICS_ON
void OsgManager::updateDynamicActors(const std::string& id, const physx::PxVec3& pxpos) {
	auto itr = physicsSphere.find(id);
	osg::Vec3 pos = osg::Vec3(pxpos.x, pxpos.y, pxpos.z);
	if (itr != physicsSphere.end()) {
		osg::Matrix projMatrix;
		projMatrix.makeIdentity();
		projMatrix.setTrans(pos);
		itr->second.second->setMatrix(projMatrix);
	}
	else {
		std::cout << __FUNCTION__ << " " << id.c_str() << " not found!" << std::endl;
	}
}
#endif
void OsgManager::showFileModel(const char* pName, bool bPointsOnly) {
	if (bPointsOnly) {
		osg::ref_ptr<osg::Geometry> geom = loadPointSet(pName);
		sceneSwitch->addChild(geom);
		return;
	}
	osg::ref_ptr<osg::Geometry> geom = loadModelFile(pName);
	sceneSwitch->addChild(geom);
	return;

	clear();

	osg::ref_ptr<osg::Node> node = getModelNode(pName, bPointsOnly);
	rootGeomTerrain->addChild(node);
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

bool OsgManager::Collaborate(const char* pName) {
	if (!bInitCollaborate) {
		pCollaborateRoot = new osg::Group;

		// add random model
		{
			ifs.open("terrain.txt");
			std::string line;
			std::getline(ifs, line);
			int nDim = std::atoi(line.c_str());
			matTerrain.resize(nDim, std::vector<float>(nDim, 0));
			float scale = 50.f;
			for (int i = 0; i < nDim; ++i) {
				std::getline(ifs, line);
				std::vector<std::string> res = splitString(line, ' ');
				if (res.size() < nDim) continue;
				for (int j = 0; j < nDim; ++j) {
					matTerrain[i][j] = std::atof(res[j].c_str());

					osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
					hints->setDetailRatio(2.0f);
					osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable(new osg::Box(osg::Vec3(i - halfDim, j - halfDim, matTerrain[i][j] / 2.f), 1, 1, matTerrain[i][j]), hints.get());
					shape->setColor(osg::Vec4(matTerrain[i][j] / scale, matTerrain[i][j] / scale, matTerrain[i][j] / scale, 1.0f));

					pCollaborateRoot->addChild(shape);
				}
			}
			ifs.close();
		}

		sceneSwitch->addChild(pCollaborateRoot);
		geom1 = createPlanet(1.f, osg::Vec4(0.f, 0.f, 1.f, 1.f));
		geom2 = createPlanet(1.f, osg::Vec4(1.f, 0.f, 0.f, 1.f));
		transform1 = new osg::MatrixTransform;
		transform2 = new osg::MatrixTransform;
		osg::Matrix projMatrix;
		projMatrix.makeIdentity();
		projMatrix.setTrans(osg::Vec3(0, 0, 0));
		transform1->setMatrix(projMatrix);
		transform1->addChild(geom1);
		pCollaborateRoot->addChild(transform1);

		projMatrix.setTrans(osg::Vec3(0, 0, 0));
		transform2->setMatrix(projMatrix);
		transform2->addChild(geom2);
		pCollaborateRoot->addChild(transform2);

		ifs.open(pName);

		bInitCollaborate = true;
	}
	
	if (ifs.fail() || ifs.eof()) {
		std::cout << " failed or end " << std::endl;
		bInitCollaborate = false;
		return false;
	}
	std::string line;
	std::getline(ifs, line);
	std::vector<std::string> res = splitString(line);
	if (res.size() < 4) return true;

	osg::Matrix projMatrix;
	int x = (std::floor)(std::stof(res[0])), y = (std::floor)(std::stof(res[1]));
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= matTerrain.size()) x = matTerrain.size() - 1;
	if (y >= matTerrain[x].size()) y = matTerrain[x].size() - 1;
	projMatrix.setTrans(osg::Vec3(std::stof(res[0]) - halfDim, std::stof(res[1]) - halfDim, matTerrain[x][y] + 0.5f));
	transform1->setMatrix(projMatrix);
	x = (std::floor)(std::stof(res[2])), y = (std::floor)(std::stof(res[3]));
	projMatrix.setTrans(osg::Vec3(std::stof(res[2]) - halfDim, std::stof(res[3]) - halfDim, matTerrain[x][y] + 0.5f));
	transform2->setMatrix(projMatrix);
	return true;
}

osg::Matrix getPose(const std::string& sPoseName) {
	
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

#include "commonGeometry/delaunay.h"



void OsgManager::showModelWithShader(osg::ref_ptr<osg::Geometry> pGeom) {
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX, "glsl/vshader_demo.glsl"));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, "glsl/fshader_demo.glsl"));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	// uniform
	osg::Uniform* mvpUniform = ss->getOrCreateUniform("mvpMat", osg::Uniform::FLOAT_MAT4);
	mvpUniform->setUpdateCallback(new MVPCallback(pviewer->getCamera()));
	osg::Uniform* viewUni = ss->getOrCreateUniform("viewMatInv", osg::Uniform::FLOAT_MAT4);
	viewUni->setUpdateCallback(new ViewCallback(pviewer->getCamera()));
	osg::Uniform* lightPosUni = ss->getOrCreateUniform("lightPos", osg::Uniform::FLOAT_VEC3);
	lightPosUni->setUpdateCallback(new LightPosCallback(pviewer->getCamera()));

	ss->setTextureAttributeAndModes(0, createTexture("glsl/basecolor.bmp"));
	ss->setTextureAttributeAndModes(1, createTexture("glsl/normal.bmp"));
	ss->addUniform(new osg::Uniform("textureMap", 0));
	ss->addUniform(new osg::Uniform("normalMap", 1));

	// in 
	program->addBindAttribLocation("o_vertex", 1);
	program->addBindAttribLocation("o_normal", 2);
	program->addBindAttribLocation("o_texcoord", 3);

	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();
	osg::Vec3Array* narray = (osg::Vec3Array*)pGeom->getNormalArray();
	osg::Vec2Array* tarray = (osg::Vec2Array*)pGeom->getTexCoordArray(0);
	if (tarray == nullptr) {
		tarray = new osg::Vec2Array;
		for (int i = 0; i < varray->size(); ++i) {
			tarray->push_back(osg::Vec2(rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX));
		}
	}
	pGeom->setVertexAttribArray(3, tarray, osg::Array::Binding::BIND_PER_VERTEX);

	pGeom->setVertexAttribArray(1, varray, osg::Array::Binding::BIND_PER_VERTEX);
	pGeom->setVertexAttribArray(2, narray, osg::Array::Binding::BIND_PER_VERTEX);

	// show
	//setWireFrame(pGeom->getOrCreateStateSet(), ShowType::SHOW_FRONT_AND_BACK);
	sceneSwitch->addChild(pGeom);

	if (false) { // show wireframe
		osg::ref_ptr<osg::Geometry> geomWireframe = new osg::Geometry(*pGeom.get(), osg::CopyOp::SHALLOW_COPY);
		setWireFrame(geomWireframe->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);
		sceneSwitch->addChild(geomWireframe);
	}
}

void OsgManager::reconstructFrame(const std::string& sFilePath) {
	// sFilePath = "E:/dataset/thermocolorlab"; // only for test 
	// for each frame: scan%03d.obj -- pointcloud; scan%03d.pose -- 6D pose (position.xyz rotation.xyz).
	int stpos = 0, endpos = 9;
	char cIndex[10] = { 0 };
	float degree = 20.f; // 40 degree rotation between frames
	for (int idx = stpos; idx < endpos; ++idx) {
		sprintf(cIndex, "%03d", idx);

		osg::ref_ptr<osg::Geometry> pGeom = loadModelFile(sFilePath + "/scan" + std::string(cIndex) + ".obj");
		osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

		std::string sPoseName = sFilePath + "/scan" + std::string(cIndex) + ".pose";
		osg::Matrix matTranslate = getPose(sPoseName);

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

		setWireFrame(pTargetGeom->getOrCreateStateSet());

		sceneSwitch->addChild(pTargetGeom);
		//showModelWithShader(pTargetGeom);
	}
}

osg::ref_ptr<osg::Geometry> getSingleGeom(std::string sFilePath, std::string cIndex) {
	osg::ref_ptr<osg::Geometry> pGeom = loadModelFile(sFilePath + "/scan" + std::string(cIndex) + ".obj");
	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();
	osg::Vec4Array* vColorArray = (osg::Vec4Array*)pGeom->getColorArray();

	std::string sPoseName = sFilePath + "/scan" + std::string(cIndex) + ".pose";
	osg::Matrix matTranslate = getPose(sPoseName);

	osg::ref_ptr<osg::Vec3Array> vTargetVertex = new osg::Vec3Array;
	osg::ref_ptr< osg::Vec4Array> vTargetColor = new osg::Vec4Array;
	for (int i = 0; i < varray->size(); ++i) {
		osg::Vec3 vRotPoint = matTranslate.preMult(varray->at(i));
		vTargetVertex->push_back(vRotPoint);
		vTargetColor->push_back(vColorArray->at(i));
	}
	pGeom->setVertexArray(vTargetVertex);
	pGeom->setColorArray(vTargetColor, osg::Array::BIND_PER_VERTEX);
	pGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, vTargetVertex->size()));
	return pGeom.release();
}

bool OsgManager::reconstructFrameStep(std::string sFilePath) {
	sFilePath = "E:/dataset/thermocolorlab";

	char cIndex[10] = { 0 };
	float degree = 20.f; // 40 degree rotation between frames

	//if (curIdx == -1) {
	//	sceneSwitch->removeChildren(0, sceneSwitch->getNumChildren());

	//	sprintf(cIndex, "%03d", 0);
	//	osg::ref_ptr<osg::Geometry> pGeom = getSingleGeom(sFilePath, std::string(cIndex));

	//	sceneSwitch->addChild(pGeom);
	//	curIdx++;
	//	return true;
	//}

	sprintf(cIndex, "%03d", curIdx);

	osg::ref_ptr<osg::Geometry> pGeom = loadModelFile(sFilePath + "/scan" + std::string(cIndex) + ".obj");

	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

	std::string sPoseName = sFilePath + "/scan" + std::string(cIndex) + ".pose";
	osg::Matrix matTranslate = getPose(sPoseName);

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
		vTargetNormal->at(index0).normalize();
		vTargetNormal->at(index1).normalize();
		vTargetNormal->at(index2).normalize();
	}

	pTargetGeom->setVertexArray(vTargetVertex);
	pTargetGeom->setNormalArray(vTargetNormal, osg::Array::Binding::BIND_PER_VERTEX);
	pTargetGeom->setColorArray(vTargetColor, osg::Array::Binding::BIND_PER_VERTEX);
	pTargetGeom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, vTargetIndices.size(), vTargetIndices.data()));

	setWireFrame(pTargetGeom->getOrCreateStateSet());

	//int nNumChild = sceneSwitch->getNumChildren();
	//sceneSwitch->removeChild(nNumChild - 1, 1);
	sceneSwitch->addChild(pTargetGeom);

	if (curIdx >= 8) {
		curIdx = -1;
		return false;
	}
	curIdx++;
	if (0) {
		
		sprintf(cIndex, "%03d", curIdx);
		osg::ref_ptr<osg::Geometry> pNextGeom = getSingleGeom(sFilePath, std::string(cIndex));
		sceneSwitch->addChild(pNextGeom);
	}

	progress = (curIdx + 1.f) / 8.f;
	return true;
}
