#include "vertexData.h"

#include <iostream>

osg::Node* readPlySpecial(const std::string& filename) 
{
    ply::VertexData vertexData;
    osg::Node* node = vertexData.readPlyFile(filename.c_str());

    if (node)
        return node;

    return nullptr;
}
