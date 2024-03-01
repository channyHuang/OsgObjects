#include "shaderParams.h"

ShaderParams* ShaderParams::instance = nullptr;


void ShaderParams::show_01(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer) {
	std::string vShader = sPath + "01/shader/" + "basic.vert.glsl";
	std::string fShader = sPath + "01/shader/" + "basic.frag.glsl";

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX,vShader));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, fShader));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	//// in 
	program->addBindAttribLocation("VertexPosition", 1);
	program->addBindAttribLocation("VertexColor", 2);

	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

	osg::Vec3Array* carray = new osg::Vec3Array;
	for (int i = 0; i < varray->size(); ++i) {
		carray->push_back(osg::Vec3(rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX));
	}

	pGeom->setVertexAttribArray(1, varray, osg::Array::Binding::BIND_PER_VERTEX);
	pGeom->setVertexAttribArray(2, carray, osg::Array::Binding::BIND_PER_VERTEX);

	osg::Uniform* mvpUniform = ss->getOrCreateUniform("mvpMat", osg::Uniform::FLOAT_MAT4);
	mvpUniform->setUpdateCallback(new MatrixCallback(pviewer));

	osg::Group* pGroup = dynamic_cast<osg::Group*>(pviewer->getSceneData());
	if (pGroup) {
		pGroup->addChild(pGeom.get());
	}
}

void ShaderParams::show_02(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer) {
	std::string vShader = sPath + "02/shader/" + "basic_uniform.vert.glsl";
	std::string fShader = sPath + "02/shader/" + "basic_uniform.frag.glsl";

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX, vShader));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, fShader));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	//// in 
	program->addBindAttribLocation("VertexPosition", 1);
	program->addBindAttribLocation("VertexColor", 2);

	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

	osg::Vec3Array* carray = new osg::Vec3Array;
	for (int i = 0; i < varray->size(); ++i) {
		carray->push_back(osg::Vec3(rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX));
	}

	pGeom->setVertexAttribArray(1, varray, osg::Array::Binding::BIND_PER_VERTEX);
	pGeom->setVertexAttribArray(2, carray, osg::Array::Binding::BIND_PER_VERTEX);

	osg::Uniform* mvpUniform = ss->getOrCreateUniform("RotationMatrix", osg::Uniform::FLOAT_MAT4);
	mvpUniform->setUpdateCallback(new MatrixCallback(pviewer));

	osg::Group* pGroup = dynamic_cast<osg::Group*>(pviewer->getSceneData());
	if (pGroup) {
		pGroup->addChild(pGeom.get());
	}
}

void ShaderParams::show_02_1(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer) {
	if (0) {
		std::string vShader = sPath + "02/shader/" + "basic_uniformblock.vert.glsl";
		std::string fShader = sPath + "02/shader/" + "basic_uniformblock.frag.glsl";
	}

	std::string vShader = sPath + "02/shader/" + "basic_uniformblock_41.vert.glsl";
	std::string fShader = sPath + "02/shader/" + "basic_uniformblock_41.frag.glsl";

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX, vShader));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, fShader));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	//// in 
	program->addBindAttribLocation("VertexPosition", 1);
	program->addBindAttribLocation("VertexColor", 2);

	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

	osg::Vec3Array* carray = new osg::Vec3Array;
	for (int i = 0; i < varray->size(); ++i) {
		carray->push_back(osg::Vec3(rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX));
	}

	pGeom->setVertexAttribArray(1, varray, osg::Array::Binding::BIND_PER_VERTEX);
	pGeom->setVertexAttribArray(2, carray, osg::Array::Binding::BIND_PER_VERTEX);

	osg::Uniform* mvpUniform = ss->getOrCreateUniform("RotationMatrix", osg::Uniform::FLOAT_MAT4);
	mvpUniform->setUpdateCallback(new MatrixCallback(pviewer));

	program->addBindUniformBlock("BlobSettings", 0);

	GLfloat outerColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat innerColor[] = { 1.0f, 1.0f, 0.75f, 1.0f };
	GLfloat innerRadius = 0.25f, outerRadius = 0.45f;

	osg::ref_ptr<osg::FloatArray> uboarray = new osg::FloatArray;
	uboarray->push_back(*outerColor);
	uboarray->push_back(*innerColor);
	uboarray->push_back(innerRadius);
	uboarray->push_back(outerRadius);

	osg::ref_ptr<osg::UniformBufferObject> ubo = new osg::UniformBufferObject;
	uboarray->setBufferObject(ubo.get());
	osg::ref_ptr<osg::UniformBufferBinding> ubb1 = new osg::UniformBufferBinding(0, uboarray.get(), 0, 5 * sizeof(GLfloat));

	osg::Group* pGroup = dynamic_cast<osg::Group*>(pviewer->getSceneData());
	if (pGroup) {
		pGroup->addChild(pGeom.get());
	}
}

void ShaderParams::show_02_2(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer) {
	std::string vShader = sPath + "02/shader/" + "separable.vert.glsl";
	std::string fShader = sPath + "02/shader/" + "separable2.frag.glsl"/*"separable1.frag.glsl"*/;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX, vShader));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, fShader));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	//// in 
	program->addBindAttribLocation("VertexPosition", 1);
	program->addBindAttribLocation("VertexColor", 2);

	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

	osg::Vec3Array* carray = new osg::Vec3Array;
	for (int i = 0; i < varray->size(); ++i) {
		carray->push_back(osg::Vec3(rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX));
	}

	pGeom->setVertexAttribArray(1, varray, osg::Array::Binding::BIND_PER_VERTEX);
	pGeom->setVertexAttribArray(2, carray, osg::Array::Binding::BIND_PER_VERTEX);

	osg::Uniform* mvpUniform = ss->getOrCreateUniform("RotationMatrix", osg::Uniform::FLOAT_MAT4);
	mvpUniform->setUpdateCallback(new MatrixCallback(pviewer));
	ss->addUniform( new osg::Uniform("uColorMask", osg::Vec3(1.f, 1.f, 1.f)));

	osg::Group* pGroup = dynamic_cast<osg::Group*>(pviewer->getSceneData());
	if (pGroup) {
		pGroup->addChild(pGeom.get());
	}
}

void ShaderParams::show_03(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pviewer) {
	std::string vShader = sPath + "03/shader/" + "diffuse.vert.glsl";
	std::string fShader = sPath + "03/shader/" + "diffuse.frag.glsl";

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX, vShader));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, fShader));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	//// in 
	program->addBindAttribLocation("VertexPosition", 0);
	program->addBindAttribLocation("VertexColor", 1);

	osg::Vec3Array* varray = (osg::Vec3Array*)pGeom->getVertexArray();

	osg::Vec3Array* carray = new osg::Vec3Array;
	for (int i = 0; i < varray->size(); ++i) {
		carray->push_back(osg::Vec3(rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX, rand() * 1.f / RAND_MAX));
	}

	pGeom->setVertexAttribArray(0, varray, osg::Array::Binding::BIND_PER_VERTEX);
	pGeom->setVertexAttribArray(1, carray, osg::Array::Binding::BIND_PER_VERTEX);

	osg::Uniform* viewUniform = ss->getOrCreateUniform("ModelViewMatrix", osg::Uniform::FLOAT_MAT4);
	viewUniform->setUpdateCallback(new MatrixCallback(pviewer));
	osg::Uniform* normalUniform = ss->getOrCreateUniform("NormalMatrix", osg::Uniform::FLOAT_MAT4);
	normalUniform->setUpdateCallback(new MatrixCallback(pviewer, MatrixType::Normal));
	osg::Uniform* projUniform = ss->getOrCreateUniform("ProjectionMatrix", osg::Uniform::FLOAT_MAT4);
	projUniform->setUpdateCallback(new MatrixCallback(pviewer, MatrixType::Project));
	osg::Uniform* mvpUniform = ss->getOrCreateUniform("MVP", osg::Uniform::FLOAT_MAT4);
	mvpUniform->setUpdateCallback(new MatrixCallback(pviewer, MatrixType::MVP));

	ss->addUniform(new osg::Uniform("Kd", osg::Vec3(0.9f, 0.5f, 0.3f)));
	ss->addUniform(new osg::Uniform("Ld", osg::Vec3(1.0f, 1.0f, 1.0f)));
	ss->addUniform(new osg::Uniform("LightPosition", osg::Vec4(5.0f, 5.0f, 2.0f, 1.0f)));

	osg::Group* pGroup = dynamic_cast<osg::Group*>(pviewer->getSceneData());
	if (pGroup) {
		pGroup->addChild(pGeom.get());
	}
}