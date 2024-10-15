#pragma once

#include <unordered_map>
#include <mutex>

#include <osgViewer/Viewer>
#include <osg/Group>
#include <osg/Switch>
#include <osg/Program>
#include <osgDB/ReadFile>
#include <osg/LineWidth>
#include <osgViewer/Renderer>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/BindImageTexture>
#include <osgText/Text>
#include <osgDB/WriteFile>
#include <osg/AutoTransform>

#include "commonOsg/osgManagerBase.h"
#include "commonOsg/commonOsg.h"

class OsgManager : public OsgManagerBase
{
public:
	static OsgManager* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new OsgManager();
		}
		return m_pInstance;
	}

	virtual ~OsgManager();

	void reconFrameDataset(const std::string& sDatasetPath = "/home/channy/Documents/datasets_recon/thermocolorlab");
	void reconFrame(const std::string& sDatasetPath);

protected:
	static OsgManager* m_pInstance;

protected:
	OsgManager();

	osg::ref_ptr<osg::Group> m_pRootGeomDistance = nullptr;
	std::vector<osg::Vec3> m_vPickPoints;
	osg::ref_ptr<osg::Group> m_pMeshGroup = nullptr;

private:
	int m_nDatasetPos = 0;
};
