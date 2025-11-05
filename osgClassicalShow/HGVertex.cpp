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
    pVertices->push_back(osg::Vec3( 0, 1, 0));
    pVertices->push_back(osg::Vec3( 1, 1, 0));
    pVertices->push_back(osg::Vec3( 1, 0, 0));

    pVertices->push_back(osg::Vec3( 0, 0, 1));
    pVertices->push_back(osg::Vec3( 1, 0, 1));
    pVertices->push_back(osg::Vec3( 1, 1, 1));
    pVertices->push_back(osg::Vec3( 0, 1, 1));
}

void generateCubeFace(osg::ref_ptr<osg::DrawElementsUInt> &pTriangles, size_t nStart = 0, size_t nEnd = 36) {
    std::vector<unsigned int> vIndex = {0, 1, 2, 0, 2, 3, //rg
                                0, 3, 5, 0, 5, 4, // rb
                                0, 4, 7, 0, 7, 1, // gb
                                6, 7, 4, 6, 4, 5, 
                                6, 2, 1, 6, 1, 7,
                                6, 5, 3, 6, 3, 2
                            };

    assert(vIndex.size() == 36);
    if (!pTriangles.valid()) {
        pTriangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    } else {
        pTriangles->clear();
    }
    for (size_t i = nStart; i < std::min(nEnd, vIndex.size()); ++i) {
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
    osg::ref_ptr<osg::Image> pImage;
    if (!sTextureFile.empty() && (pImage = osgDB::readRefImageFile(sTextureFile)) != nullptr) {
        osg::Texture2D *pTexture = new osg::Texture2D;
        pTexture->setImage(pImage.get());

        pTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);

        pTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        pTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        pTexture->setResizeNonPowerOfTwoHint(false);

        return pTexture;
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

osg::Node* HGVertex::showBoxWithTexture() {
    // cub verteices
    generateCubeVertex(m_pVertices);
    generateCubeFace(m_pTriangles);
    generateCubeTexcoord(m_pTexcoords);
    
    //geom
    osg::Geometry* pGeom  =  new osg::Geometry;
    pGeom->setVertexArray(m_pVertices.get());
    if (m_pTriangles.valid() && m_pTriangles->size() > 0) {
        pGeom->addPrimitiveSet(m_pTriangles.get());
    }
    if (m_pTexcoords.valid()) {
        pGeom->setTexCoordArray(0, m_pTexcoords.get());
    }

    osg::ref_ptr<osg::UIntArray> texCoordIndices = new osg::UIntArray;
    for (size_t i = 0; i < m_pTexcoords->size(); ++i) {
        texCoordIndices->push_back(i);
    }

    osg::StateSet *pStateset = pGeom->getOrCreateStateSet();
    pStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    pStateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // light
    pStateset->setMode(GL_LIGHT0, osg::StateAttribute::ON);

    // texture
    std::vector<std::string> vTextureFiles = {"../data/texture/Marble.bmp", "../data/texture/Concrete.bmp"};

    for (size_t i = 0; i < vTextureFiles.size(); ++i) {
        auto pTexture = generateTexture(vTextureFiles[i]);

        if (pTexture != nullptr)
            pStateset->setTextureAttributeAndModes(i, pTexture, osg::StateAttribute::ON);
    }
    
    // materials
    auto pMaterial = generateMaterial();
    if (pMaterial != nullptr) 
        pStateset->setAttributeAndModes(pMaterial, osg::StateAttribute::ON);

    osg::Geode* pGeode = new osg::Geode;
    pGeode->addDrawable(pGeom);

    return pGeode;
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

// ----------------------------------------------------------------------------------------------------

osg::Geometry* generateGeometry(osg::ref_ptr<osg::Vec3Array> pVertices,
                                osg::ref_ptr<osg::DrawElementsUInt> pTriangles, 
                                osg::ref_ptr<osg::Vec2Array> pTexcoords,
                                size_t nIndex = 0,
                                const std::string &sTexFile = "../data/texture/Marble.bmp") {
    osg::Geometry* pGeom = new osg::Geometry;
    pGeom->setVertexArray(pVertices.get());
    if (pTriangles.valid() && pTriangles->size() > 0) {
        pGeom->addPrimitiveSet(pTriangles.get());
    }
    if (pTexcoords.valid()) {
        pGeom->setTexCoordArray(nIndex, pTexcoords.get());
    }

    osg::StateSet *pStateset = pGeom->getOrCreateStateSet();
    pStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    pStateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // texture
    auto pTexture = generateTexture(sTexFile);
    if (pTexture != nullptr)
        pStateset->setTextureAttributeAndModes(nIndex, pTexture, osg::StateAttribute::ON);

    // materials
    auto pMaterial = generateMaterial();
    if (pMaterial != nullptr) 
        pStateset->setAttributeAndModes(pMaterial, osg::StateAttribute::ON);

    return pGeom;
}


osg::Node* HGVertex::showBoxWithMultiTexture() {
    generateCubeVertex(m_pVertices);
    osg::ref_ptr<osg::DrawElementsUInt> pTriangles1, pTriangles2;
    generateCubeFace(pTriangles1, 0, 18);
    generateCubeFace(pTriangles2, 18, 36);
    osg::ref_ptr<osg::Vec2Array> pTexcoords1, pTexcoords2;
    generateCubeTexcoord(pTexcoords1);
    generateCubeTexcoord(pTexcoords2);
    osg::Geometry* pGeom1 = generateGeometry(m_pVertices, pTriangles1, pTexcoords1);
    osg::Geometry* pGeom2 = generateGeometry(m_pVertices, pTriangles2, pTexcoords2, 1, "../data/texture/Wood.bmp");

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(pGeom1);
    geode->addDrawable(pGeom2);

    return geode;
}

void generateCubeTexcoordQuat(osg::ref_ptr<osg::Vec2Array> &pTexcoord, osg::ref_ptr<osg::DrawElementsUInt> &pTriangles) {
    if (!pTexcoord.valid()) {
        pTexcoord = new osg::Vec2Array;
    } else {
        pTexcoord->clear();
    }

    std::vector<osg::Vec2> vCoord = {osg::Vec2(0.0f, 0.0f), osg::Vec2(1.0f, 0.0f), osg::Vec2(1.0f, 1.0f),
                                    osg::Vec2(0.0f, 0.0f), osg::Vec2(1.0f, 1.0f), osg::Vec2(0.0f, 1.0f)
                                    };

    for (size_t i = 0; i < 8; ++i) {
        pTexcoord->push_back(vCoord[i % vCoord.size()]);
    }
    size_t i = 0;
    for (size_t j = 0; j < pTriangles->size(); ++j) {
        (*pTexcoord)[pTriangles->at(j)] = vCoord[i++];
    }
}

osg::Node *HGVertex::showBoxWithRightTexture() {
    std::vector<std::string> vTexFiles = {"../data/texture/basecolor.bmp", 
                                        "../data/texture/Concrete.bmp", 
                                        "../data/texture/posx.bmp", 
                                        "../data/texture/normal.bmp", 
                                        "../data/texture/rockwall.bmp", 
                                        "../data/texture/Wood.bmp"};

    osg::Geode* geode = new osg::Geode;

    generateCubeVertex(m_pVertices);
    for (size_t i = 0; i < 6; ++i) {
        osg::ref_ptr<osg::DrawElementsUInt> pTriangles;
        generateCubeFace(pTriangles, i * 6, (i + 1) * 6);
        osg::ref_ptr<osg::Vec2Array> pTexcoords;
        generateCubeTexcoordQuat(pTexcoords, pTriangles);

        osg::Geometry* pGeom = generateGeometry(m_pVertices, pTriangles, pTexcoords, 0, vTexFiles[i]);

        geode->addDrawable(pGeom);
    }
    return geode;

}
