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

class HGVertex {
public:
    HGVertex();

    osg::Node* originBox();
    osg::LightSource* light();

public:
    osg::ref_ptr<osg::Vec3Array>   _vertices;
    osg::ref_ptr<osg::Vec2Array>   _texcoord;

    osg::ref_ptr<osg::DrawElementsUInt> _triangles;
};