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

class OsgManager
{
public:
	static OsgManager* getInstance() {
		if (instance == nullptr) {
			instance = new OsgManager();
		}
		return instance;
	}

	~OsgManager();

	void switchScene();
	void setViewer(osgViewer::Viewer& viewer);
	void clearPick();
	void showPick(osg::Vec3& p);

	void obj2osgt(const std::string& str);
	void showModelObj(const std::string& str, const std::string& sTex, bool bUseOsgLoad = true);
	void showModelOsgbLOD(const std::string& str, const std::string& sTex);
	void showModelUsingAssimp(std::string sFileName, std::string sTextureName);

private:
	OsgManager();
	
private:
	static OsgManager* instance;

	osg::ref_ptr<osg::Group> root = nullptr;
	osg::ref_ptr<osg::Group> sunLight = nullptr;
	osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
	osg::ref_ptr<osg::Switch> sceneSwitch;
	osg::ref_ptr<osg::Group> rootGeomDistance = nullptr;
	int sceneMaxIdx = 0;
	int sceneIdx = 0;
	std::vector<osg::Vec3> m_vPickPoints;
};

