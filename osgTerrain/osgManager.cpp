#include "osgManager.h"

#include <filesystem>

OsgManager::OsgManager() {
	m_pRootGroup->addChild(createSkyBox());
	m_pRootGroup->addChild(createAxis());

	m_pRootGeomTerrain = new osg::Group;
	m_pRootWireTerrain = new osg::Group;
	m_pSceneSwitcher->addChild(m_pRootGeomTerrain);
	m_pSceneSwitcher->addChild(m_pRootWireTerrain);

	m_pSceneSwitcher->setAllChildrenOn();

	m_pBrush = std::make_shared<VoxelBrush>(TerrainManager::getInstance(), VoxelMap::getInstance());

	m_pRootWireTerrain->setNodeMask(0);
	m_pGlobalStateSet = createGlobalTexture();
}

OsgManager::~OsgManager() {
	clear();
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

	updateShow();
}

void OsgManager::onOffWireframe() {
	if (m_pRootWireTerrain.valid()) {
		unsigned int mask = m_pRootWireTerrain->getNodeMask();
		if (mask == 0)
			m_pRootWireTerrain->setNodeMask(0xffffffff);
		else 
			m_pRootWireTerrain->setNodeMask(0);
	}
}

osg::ref_ptr<osg::StateSet> OsgManager::createGlobalTexture() {
	std::string sTexturePath = "../data/texture";
	std::vector<std::filesystem::path> sMaterialFiles;
	for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::absolute(sTexturePath))) {
        if (entry.is_regular_file()) {
            std::string sFileName = entry.path().filename().string();
            if (!sFileName.starts_with("material-")) continue;
			sMaterialFiles.emplace_back(entry.path());
		}
	}
	std::sort(sMaterialFiles.begin(), sMaterialFiles.end());

	osg::ref_ptr<osg::Texture2DArray> pTextureArr = new osg::Texture2DArray;
	pTextureArr->setTextureSize(1024, 1024, sMaterialFiles.size()); 
	
	osg::ref_ptr<osgDB::Options> options = new osgDB::Options;
	options->setOptionString("RGBA");
	for (int i = 0; i < sMaterialFiles.size(); ++i) {
		osg::ref_ptr<osg::Image> pImage = osgDB::readImageFile(sMaterialFiles[i], options.get());
		if (pImage) {
			if (pImage->getPixelFormat() != GL_RGBA || pImage->getDataType() != GL_UNSIGNED_BYTE) {
            	osg::ref_ptr<osg::Image> pConvertedImg = new osg::Image;
            	pConvertedImg->allocateImage(pImage->s(), pImage->t(), 1, GL_RGBA, GL_UNSIGNED_BYTE);
				osg::copyImage(pImage.get(), 0, 0, 0, pImage->s(), pImage->t(), 1,
										pConvertedImg.get(), 0, 0, 0, false);
				pImage = pConvertedImg;
			}
			pTextureArr->setImage(i, pImage);
		}
	}
	pTextureArr->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	pTextureArr->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

	osg::ref_ptr<osg::StateSet> pGlobalStateSet = new osg::StateSet;
	pGlobalStateSet->setTextureAttributeAndModes(0, pTextureArr.get());
	pGlobalStateSet->addUniform(new osg::Uniform("textureArray", 0));

	// add shader
    osg::ref_ptr<osg::Program> pShaderProgram = new osg::Program;
    pShaderProgram->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, "../data/shaders/surface.vert"));
    pShaderProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, "../data/shaders/surface.frag"));
	pGlobalStateSet->setAttributeAndModes(pShaderProgram, osg::StateAttribute::ON);

	return pGlobalStateSet;
}

// osg::ref_ptr<osg::Geometry> OsgManager::getSurfaceGeometry(const Arrays& surface, osg::Vec3 color) {
// 	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
// 	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
// 	for (auto& pt : surface.positions) {
// 		vertices->push_back(osg::Vec3(pt.x, pt.y, pt.z));
// 	}
// 	geom->setVertexArray(vertices.get());

// 	osg::ref_ptr<osg::Vec3Array> pNormals = new osg::Vec3Array;
// 	for (auto& n : surface.normals) {
// 		pNormals->push_back(osg::Vec3(n.x, n.y, n.z));
// 	}
// 	// geom->setNormalArray(pNormals, osg::Array::BIND_PER_VERTEX);
// 	geom->setVertexAttribArray(1, pNormals, osg::Array::BIND_PER_VERTEX);
// 	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	
// 	// could not use osg::UIntArray here, will not be translated to shader correctly
// 	osg::ref_ptr<osg::FloatArray> texIndices[8];
// 	for (size_t i = 0; i < 8; ++i) {
// 		texIndices[i] = new osg::FloatArray;
// 		texIndices[i]->resize(surface.materials.size(), 0);
// 	}
// 	size_t nIdxMaterial = 0; 
// 	for (uint32_t eMaterial : surface.materials) {
// 		size_t nIdxTex = 0;
// 		for (size_t i = 0; i < MaterialType::Material_MAX; ++i) {
// 			if ( (eMaterial & (1 << i)) ) {
// 				texIndices[nIdxTex]->at(nIdxMaterial) = i * 1.0;
// 				nIdxTex++;
// 			}
// 		}
// 		nIdxMaterial++;
// 			}

// 	for (size_t i = 0; i < 8; ++i) {
// 		geom->setVertexAttribArray(2 + i, texIndices[i], osg::Array::BIND_PER_VERTEX);
// 		geom->setVertexAttribBinding(2 + i, osg::Geometry::BIND_PER_VERTEX);
// 	}

// 	geom->setStateSet(m_pGlobalStateSet.get()); 

// 	geom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, surface.indices.size(), surface.indices.data()));
// 	return geom;
// }

osg::ref_ptr<osg::Geometry> OsgManager::getSurfaceGeometry(const Arrays& surface, osg::Vec3 color) {
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	for (auto& pt : surface.positions) {
		vertices->push_back(osg::Vec3(pt.x, pt.y, pt.z));
	}
	geom->setVertexArray(vertices.get());

	osg::ref_ptr<osg::Vec3Array> pNormals = new osg::Vec3Array;
	for (auto& n : surface.normals) {
		pNormals->push_back(osg::Vec3(n.x, n.y, n.z));
	}
	// geom->setNormalArray(pNormals, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribArray(1, pNormals, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	
	// could not use osg::UIntArray here, will not be translated to shader correctly
	osg::ref_ptr<osg::Vec4Array> pWeights[4];
	for (size_t i = 0; i < 4; ++i) {
		pWeights[i] = new osg::Vec4Array;
		pWeights[i]->resize(surface.materials.size(), osg::Vec4(0, 0, 0, 0));
	}
	size_t nIdxMaterial = 0; 
	for (uint32_t eMaterial : surface.materials) {
		int count = 0;
		for (size_t i = 0; i < MaterialType::Material_MAX; ++i) {
			if ( (eMaterial & (1 << i)) ) {
				count++;
			}
		}
		double weight = 1. / count;
		size_t nIdxTex = 0;
		for (size_t i = 1; i < MaterialType::Material_MAX; ++i) {
			if ( (eMaterial & (1 << i)) ) {
				pWeights[(i - 1) / 4]->at(nIdxMaterial)[(i - 1) % 4] = weight;
			}
		}
		nIdxMaterial++;
	}

	for (size_t i = 0; i < 4; ++i) {
		geom->setVertexAttribArray(2 + i, pWeights[i], osg::Array::BIND_PER_VERTEX);
		geom->setVertexAttribBinding(2 + i, osg::Geometry::BIND_PER_VERTEX);
	}

	geom->setStateSet(m_pGlobalStateSet.get()); 

	geom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, surface.indices.size(), surface.indices.data()));
	return geom;
}

osg::ref_ptr<osg::Geometry> OsgManager::getSurfaceGeometryFrame(const Arrays& surface, osg::Vec3 color) {
	osg::ref_ptr<osg::Geometry> pGeom = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> pVertices = new osg::Vec3Array;
	for (auto& pt : surface.positions) {
		pVertices->push_back(osg::Vec3(pt.x, pt.y, pt.z));
	}
	pGeom->setVertexArray(pVertices.get());

	osg::ref_ptr<osg::Vec4Array> pColors = new osg::Vec4Array;
	pColors->push_back(osg::Vec4(color.x(), color.y(), color.z(), 1.f));
	pGeom->setColorArray(pColors.get(), osg::Array::BIND_OVERALL);

	pGeom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, surface.indices.size(), surface.indices.data()));
	return pGeom;
}

// update each block when mesh generate done
void OsgManager::updateTerrain(const Arrays& surface, Vector3i pos) {
	osg::ref_ptr<osg::Geometry> pGeom = getSurfaceGeometry(surface);
	m_stBoundingSphere.expandBy(pGeom->getBound());
	
	osg::StateSet* pStateSet = pGeom->getOrCreateStateSet();
	setWireFrame(pStateSet, ShowType::SHOW_FRONT_AND_BACK);

	osg::ref_ptr<osg::Geometry> pGeomFrame = getSurfaceGeometryFrame(surface, osg::Vec3(0.f, 1.f, 0.f));
	setWireFrame(pGeomFrame->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);

	{
		std::unique_lock<std::mutex> locker(mutex);
		std::unordered_map < Vector3i, std::pair<osg::ref_ptr<osg::Geometry>, osg::ref_ptr<osg::Geometry>>>::iterator scene = m_mapScene.find(pos);
		
		if (scene != m_mapScene.end()) {
			m_pRootGeomTerrain->replaceChild(scene->second.first, pGeom);
			m_pRootWireTerrain->replaceChild(scene->second.second, pGeomFrame);
		}
		else {
			m_pRootGeomTerrain->addChild(pGeom);
			m_pRootWireTerrain->addChild(pGeomFrame);
		}
		m_mapScene.insert(std::make_pair(pos, std::make_pair(pGeom, pGeomFrame)));
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

void OsgManager::setLight() {
	osg::ref_ptr<osg::LightSource> pSunLightSource = new osg::LightSource;

	osg::Light* pSunLight = pSunLightSource->getLight();
	pSunLight->setLightNum(0);
	// sunLight->setPosition(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// sunLight->setAmbient(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// //sunLight->setDiffuse(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// sunLightSource->setLight(sunLight);
	// sunLightSource->setLocalStateSetModes(osg::StateAttribute::ON);
	// sunLightSource->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

	// osg::LightModel* lightModel = new osg::LightModel;
	// lightModel->setAmbientIntensity(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// sunLightSource->getOrCreateStateSet()->setAttribute(lightModel);

	osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum(0);
    light->setPosition(osg::Vec4(-1.0f, -1.0f, -1.0f, 0.0f)); 
    light->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f)); 
    light->setDiffuse(osg::Vec4(0.9f, 0.9f, 0.9f, 1.0f)); 
    light->setSpecular(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));

    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->setLight(light);
    lightSource->setReferenceFrame(osg::LightSource::ABSOLUTE_RF);
  

	m_pRootGeomTerrain->addChild(lightSource);
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

