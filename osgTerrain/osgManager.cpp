#include "osgManager.h"

OsgManager::OsgManager() {
	// m_pRootGroup->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	m_pRootGroup->addChild(createSkyBox());
	m_pRootGroup->addChild(createAxis());

	m_pRootGeomTerrain = new osg::Group;
	// m_pRootGeomTerrain->addChild(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.1f)));
	m_pRootWireTerrain = new osg::Group;//createLight(osg::Vec3(0.f, 0.f, 5.f), 1, bs, root);
	// m_pRootWireTerrain->addChild(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.1f)));
	m_pSceneSwitcher->addChild(m_pRootGeomTerrain);
	m_pSceneSwitcher->addChild(m_pRootWireTerrain);

	m_pSceneSwitcher->setAllChildrenOn();

	m_pBrush = std::make_shared<VoxelBrush>(TerrainManager::getInstance(), VoxelMap::getInstance());
}

OsgManager::~OsgManager() {
	clear();
}

void OsgManager::init() {
	clear();
	//sunLight = createSunLight();
	//root->addChild(sunLight);
	pViewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
	eye = osg::Vec3(0, 0, 1);
	pViewer->getCamera()->setViewMatrixAsLookAt(eye, center, up);
	
}

void OsgManager::clear() {
	if (m_pRootGeomTerrain.valid()) {
		if (m_pRootGeomTerrain->getNumChildren()) {
			m_pRootGeomTerrain->removeChildren(1, m_pRootGeomTerrain->getNumChildren());
		}
	} 
	if (m_pRootWireTerrain.valid()) {
		if (m_pRootWireTerrain->getNumChildren()) {
			m_pRootWireTerrain->removeChildren(1, m_pRootWireTerrain->getNumChildren());
		}
	}
	m_mapScene.clear();
	m_pSceneSwitcher->removeChildren(2, m_pSceneSwitcher->getNumChildren());

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

// update each block when mesh generate done
void OsgManager::updateTerrain(const Arrays& surface, Vector3i pos) {
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

	{
		std::unique_lock<std::mutex> locker(mutex);
		std::unordered_map < Vector3i, std::pair<osg::ref_ptr<osg::Geometry>, osg::ref_ptr<osg::Geometry>>>::iterator scene = m_mapScene.find(pos);
		
		if (scene != m_mapScene.end()) {
			m_pRootGeomTerrain->replaceChild(scene->second.first, geom);
			m_pRootWireTerrain->replaceChild(scene->second.second, geomFrame);
		}
		else {
			m_pRootGeomTerrain->addChild(geom);
			m_pRootWireTerrain->addChild(geomFrame);
		}
		m_mapScene.insert(std::make_pair(pos, std::make_pair(geom, geomFrame)));
	}

#ifdef PHYSICS_ON
	PhysicsManager::getInstance()->addTerrain(pos, surface);
#endif
}

void OsgManager::updateShow() {
	if (!m_pRootGroup.valid()) {
		std::cerr << "-------- invalid " << std::endl;
		return;
	}
	
	if (pViewer != nullptr) {
		pViewer->setSceneData(m_pRootGroup);
	}
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

		m_pSceneSwitcher->addChild(pCollaborateRoot);
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

#include "commonGeometry/delaunay.h"

void OsgManager::showModelWithShader(osg::ref_ptr<osg::Geometry> pGeom) {
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX, "glsl/vshader_demo.glsl"));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, "glsl/fshader_demo.glsl"));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	// uniform
	osg::Uniform* mvpUniform = ss->getOrCreateUniform("mvpMat", osg::Uniform::FLOAT_MAT4);
	mvpUniform->setUpdateCallback(new MVPCallback(pViewer->getCamera()));
	osg::Uniform* viewUni = ss->getOrCreateUniform("viewMatInv", osg::Uniform::FLOAT_MAT4);
	viewUni->setUpdateCallback(new ViewCallback(pViewer->getCamera()));
	osg::Uniform* lightPosUni = ss->getOrCreateUniform("lightPos", osg::Uniform::FLOAT_VEC3);
	lightPosUni->setUpdateCallback(new LightPosCallback(pViewer->getCamera()));

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
	m_pSceneSwitcher->addChild(pGeom);

	if (false) { // show wireframe
		osg::ref_ptr<osg::Geometry> geomWireframe = new osg::Geometry(*pGeom.get(), osg::CopyOp::SHALLOW_COPY);
		setWireFrame(geomWireframe->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);
		m_pSceneSwitcher->addChild(geomWireframe);
	}
}

