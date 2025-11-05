#pragma once 

#include <osg/Node>
#include <osg/PrimitiveSet>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osgDB/ReadFile>
#include <osgUtil/SmoothingVisitor>
#include <osg/Material>
#include <osg/LightSource>

#include <vector>

class HGVertex {
public:
    HGVertex();

    osg::Node* showBoxWithTexture();
    osg::Node* showBoxWithMultiTexture();
    osg::Node* showBoxWithRightTexture();
    osg::LightSource* light();

public:
    osg::ref_ptr<osg::Vec3Array> m_pVertices;
    osg::ref_ptr<osg::Vec2Array> m_pTexcoords;
    osg::ref_ptr<osg::DrawElementsUInt> m_pTriangles;
};
