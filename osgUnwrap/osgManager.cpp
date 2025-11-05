#include "osgManager.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "commonOsg/commonOsg.h"

OsgManager* OsgManager::m_pInstance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
}

OsgManager::~OsgManager() {
}

GLuint OsgManager::genTexture(const unsigned char *data, int width, int height) {
	return createTexture(data, width, height);
}
