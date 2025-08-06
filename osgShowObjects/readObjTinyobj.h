#pragma once

#include "commonOsg/tiny_obj_loader.h"

const std::string sGlslPath = "/home/channy/Documents/projects/commonClass/glsl";

osg::ref_ptr<osg::Geometry> getObjTinyobj(const char* pFileName, const char* pBasePath, osgViewer::Viewer* pViewer = nullptr) {
    tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	bool triangulate = true;

    std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		pFileName, pBasePath, triangulate);

	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << "ERR: " << err << std::endl;
	}

	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return nullptr;
	}

    osg::ref_ptr<osg::Geometry> pGeom = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> vertex = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array();

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {

			vertex->push_back(osg::Vec3(
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			));

			//texcoord					
			if (index.texcoord_index >= 0) {
				texcoords->push_back(osg::Vec2(
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				));
			}

			if (index.normal_index >= 0) {
				normal->push_back(osg::Vec3(
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				));
			}
		}
	}

	pGeom->setVertexArray(vertex);
	pGeom->setNormalArray(normal);
	pGeom->setTexCoordArray(0, texcoords);

	pGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, vertex->size()));
	return pGeom;	
}

void getGlslObj(osg::ref_ptr<osg::Geometry> pGeom, osgViewer::Viewer* pViewer) {
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readRefShaderFile(osg::Shader::VERTEX, sGlslPath + "/vshader_demo.glsl"));
	program->addShader(osgDB::readRefShaderFile(osg::Shader::FRAGMENT, sGlslPath + "/fshader_demo.glsl"));

	osg::ref_ptr<osg::StateSet> ss = pGeom->getOrCreateStateSet();
	ss->setAttribute(program);

	if (pViewer != nullptr) {
		// uniform
		osg::Uniform* mvpUniform = ss->getOrCreateUniform("mvpMat", osg::Uniform::FLOAT_MAT4);
		mvpUniform->setUpdateCallback(new MVPCallback(pViewer->getCamera()));
		osg::Uniform* viewUni = ss->getOrCreateUniform("viewMatInv", osg::Uniform::FLOAT_MAT4);
		viewUni->setUpdateCallback(new ViewCallback(pViewer->getCamera()));
		osg::Uniform* lightPosUni = ss->getOrCreateUniform("lightPos", osg::Uniform::FLOAT_VEC3);
		lightPosUni->setUpdateCallback(new LightPosCallback(pViewer->getCamera()));
	}

	ss->setTextureAttributeAndModes(0, createTexture("d:/osgfile/obj/textured_output.bmp"));
	ss->addUniform(new osg::Uniform("textureMap", 0));

	// in 
	program->addBindAttribLocation("ivertex", 1);
	program->addBindAttribLocation("inormal", 2);
	program->addBindAttribLocation("itexcoord", 3);

	// pGeom->setVertexAttribArray(1, vertex, osg::Array::Binding::BIND_PER_VERTEX);
	// if (normal->size() > 0) {
	// 	pGeom->setVertexAttribArray(2, normal, osg::Array::Binding::BIND_PER_VERTEX);
	// }
	// else {
	// 	normal->push_back(osg::Vec3(0, 1, 0));
	// 	pGeom->setVertexAttribArray(2, normal, osg::Array::Binding::BIND_OVERALL);
	// }
	// pGeom->setVertexAttribArray(3, texcoords, osg::Array::Binding::BIND_PER_VERTEX);

	// if (false) { // show wireframe
	// 	osg::ref_ptr<osg::Geometry> geomWireframe = new osg::Geometry(*pGeom.get(), osg::CopyOp::SHALLOW_COPY);
	// 	setWireFrame(geomWireframe->getOrCreateStateSet(), ShowType::SHOW_WIREFRAME);
	// }
	pGeom->dirtyGLObjects();
}