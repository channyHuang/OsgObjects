#include "HGVertex.h"

#include <assert.h>

HGVertex::HGVertex() {}

void generateCubeVertex(osg::ref_ptr<osg::Vec3Array> &pVertices) {
    if (!pVertices.valid()) {
        pVertices = new osg::Vec3Array;
    } else {
        pVertices->clear();
    }
    pVertices->push_back(osg::Vec3( 0, 0, 0));
    pVertices->push_back(osg::Vec3( 1, 0, 0));
    pVertices->push_back(osg::Vec3( 1, 1, 0));
    pVertices->push_back(osg::Vec3( 0, 1, 0));

    pVertices->push_back(osg::Vec3( 0, 1, 1));
    pVertices->push_back(osg::Vec3( 1, 1, 1));
    pVertices->push_back(osg::Vec3( 1, 0, 1));
    pVertices->push_back(osg::Vec3( 0, 0, 1));
}

void generateCubeFace(osg::ref_ptr<osg::DrawElementsUInt> &pTriangles) {
    std::vector<unsigned int> vIndex = {0, 1, 2, 0, 2, 3,
                                0, 3, 4, 0, 4, 7,
                                0, 7, 6, 0, 6, 1,
                                5, 6, 7, 5, 7, 4,
                                5, 4, 3, 5, 3, 2,
                                5, 2, 1, 5, 1, 6
                            };
    assert(vIndex.size() == 36);
    if (!pTriangles.valid()) {
        pTriangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    } else {
        pTriangles->clear();
    }
    for (size_t i = 0; i < vIndex.size(); ++i) {
        pTriangles->push_back(vIndex[i]);
    }
}

void generateCubeTexcoord(osg::ref_ptr<osg::Vec2Array> &pTexcoord) {
    if (!pTexcoord.valid()) {
        pTexcoord = new osg::Vec2Array;
    } else {
        pTexcoord->clear();
    }

    std::vector<osg::Vec2> vCoord = {osg::Vec2(0.0f, 0.0f), osg::Vec2(1.0f, 0.0f), osg::Vec2(1.0f, 1.0f),
                                    osg::Vec2(0.0f, 0.0f), osg::Vec2(1.0f, 1.0f), osg::Vec2(0.0f, 1.0f)
                                    };

    for (size_t i = 0; i < 6; ++i) {
        for (size_t j = 0; j < vCoord.size(); ++j) {
            pTexcoord->push_back(vCoord[j]);
        }
    }
}

osg::Texture2D* generateTexture(const std::string &sTextureFile) {
    osg::ref_ptr<osg::Image> image;
    if (!sTextureFile.empty() && (image = osgDB::readRefImageFile(sTextureFile)) != nullptr) {
        osg::Texture2D *texture = new osg::Texture2D;
        texture->setImage(image.get());

        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        texture->setResizeNonPowerOfTwoHint(false);

        return texture;
    } 
    return nullptr;
}

osg::Material* generateMaterial() {
    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    material->setShininess(osg::Material::FRONT_AND_BACK, 32.0f);
    return material.release();
}

osg::Node* HGVertex::originBox() {
    // cub verteices
    generateCubeVertex(_vertices);
    generateCubeFace(_triangles);
    generateCubeTexcoord(_texcoord);
    
    //geom
    osg::Geometry* geom  =  new osg::Geometry;
    geom->setVertexArray(_vertices.get());
    if (_triangles.valid() && _triangles->size() > 0) {
        geom->addPrimitiveSet(_triangles.get());
    }
    if (_texcoord.valid()) {
        geom->setTexCoordArray(0, _texcoord.get());
    }

    // texture
    std::vector<std::string> textureFiles = {"../data/texture/Marble.bmp", "../data/texture/Concrete.bmp"};

    // materials
    auto pMaterial = generateMaterial();
    auto pTexture = generateTexture(textureFiles[0]);

    osg::StateSet *stateset = geom->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // light
    stateset->setMode(GL_LIGHT0, osg::StateAttribute::ON);

    if (pTexture != nullptr)
        stateset->setTextureAttributeAndModes(0, pTexture, osg::StateAttribute::ON);
    if (pMaterial != nullptr) 
        stateset->setAttributeAndModes(pMaterial, osg::StateAttribute::ON);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);

    return geode;
}

osg::LightSource* HGVertex::light() {
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum(0);
    light->setPosition(osg::Vec4(-1.0f, -1.0f, -1.0f, 0.0f)); 
    light->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f)); 
    light->setDiffuse(osg::Vec4(0.9f, 0.9f, 0.9f, 1.0f)); 
    light->setSpecular(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));

    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->setLight(light);
    lightSource->setReferenceFrame(osg::LightSource::ABSOLUTE_RF);
  

    return lightSource.release();
}