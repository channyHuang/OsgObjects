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

	void obj2osgt(const std::string& str);
	void readObjTinyobj(const std::string& str, const std::string& sTex, bool bUseOsgLoad = true);
	void loadPlyFolder(const std::string &sFolder);

	// virtual void showPick(const osg::Vec3& vPos);

protected:
	static OsgManager* m_pInstance;

protected:
	OsgManager();

	osg::ref_ptr<osg::Group> m_pMeshGroup = nullptr;
};
