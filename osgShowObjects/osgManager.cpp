#include "osgManager.h"

#include <filesystem>
#include <osg/CullFace>

#include "commonOsg/commonOsg.h"
#include "readObjTinyobj.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

OsgManager* OsgManager::m_pInstance = nullptr;

OsgManager::OsgManager() : OsgManagerBase() {
	m_pRootGeomDistance = new osg::Group;
	m_pSceneSwitcher->addChild(m_pRootGeomDistance);
}

OsgManager::~OsgManager() {
	m_pRootGeomDistance.release();
}

void OsgManager::readObjTinyobj(const std::string &sFileName, const std::string& sBasePath, bool bUseOsgLoad) {
	osg::ref_ptr<osg::Geometry> pGeom = getObjTinyobj(sFileName.c_str(), sBasePath.c_str());
	m_pSceneSwitcher->addChild(pGeom);
}

void OsgManager::obj2osgt(const std::string& str) {
	osg::ref_ptr<osg::Geometry> geom = getObjTinyobj(str.c_str(), "");
	osgDB::writeNodeFile(*geom, std::string("./hud.osgt"));
}

void OsgManager::loadPlyFolder(const std::string &sFolder) {
	fs::path path(sFolder);
    if (!fs::exists(path)) {
        std::cout << "Path not found" << sFolder << std::endl;
        return;
    }

	if (m_pMeshGroup == nullptr) {
		m_pMeshGroup = new osg::Group();
	} else {
		m_pMeshGroup->removeChildren(0, m_pMeshGroup->getNumChildren());
	}

    auto itr = fs::directory_iterator(path);
    auto end = fs::directory_iterator();
    while (itr != end) {
        auto &entry = itr;
        auto file_name = entry->path().c_str();
		std::string sFileName = std::string(file_name);

		auto pos = sFileName.rfind('.');
		if (pos == std::string::npos) {
			itr++;
			continue;
		}
		std::string ext = sFileName.substr(pos + 1);
		if (ext != "ply" && ext != "PLY") {
			itr++;
			continue;
		}

		// std::cout << sFileName << std::endl;

		osg::ref_ptr<osg::Node> pMeshNode = osgDB::readNodeFile(file_name);
		osg::ref_ptr<osg::Geode> pMeshGeode = pMeshNode->asGeode();
		if (pMeshGeode) {
			osg::ref_ptr<osg::Drawable> pMeshDrawable = pMeshGeode->getDrawable(0);
			if (pMeshDrawable) {
				osg::ref_ptr<osg::Geometry> pMeshGeometry = pMeshDrawable->asGeometry();
				if (pMeshGeometry && pMeshGeometry->getNumPrimitiveSets() > 0) {
					m_pMeshGroup->addChild(pMeshNode);
				}
			}
		}

		itr++;
	}
	// osg::ref_ptr<osg::CullFace> cullface = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);
	// m_pMeshGroup->getOrCreateStateSet()->setAttribute(cullface.get());
	// m_pMeshGroup->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

	m_pSceneSwitcher->addChild(m_pMeshGroup);
}
