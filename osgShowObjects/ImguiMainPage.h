#pragma once

#include "OsgImguiHandler.h"

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/ref_ptr>

#include "commonOsg/osgPickHandler.h"
#include "osgManager.h"

class PickDistanceHandler : public osgGA::GUIEventHandler {
public:
	PickDistanceHandler() {}

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		switch (ea.getEventType())
		{
		case(osgGA::GUIEventAdapter::PUSH):
		{
			osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
			pick(viewer, ea);
		}
		return false;

		default:
			return false;
		}
	}

	void pick(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
	{
		if (!valid) return;

		osg::Group* root = dynamic_cast<osg::Group*>(viewer->getSceneData());
		if (!root) return;

		osgUtil::LineSegmentIntersector::Intersections intersections;
		if (viewer->computeIntersections(ea, intersections))
		{
			const osgUtil::LineSegmentIntersector::Intersection& hit = *intersections.begin();

			bool handleMovingModels = false;
			const osg::NodePath& nodePath = hit.nodePath;
			for (osg::NodePath::const_iterator nitr = nodePath.begin();
				nitr != nodePath.end();
				++nitr)
			{
				const osg::Transform* transform = dynamic_cast<const osg::Transform*>(*nitr);
				if (transform)
				{
					if (transform->getDataVariance() == osg::Object::DYNAMIC) handleMovingModels = true;
				}
			}

			osg::Vec3 position = handleMovingModels ? hit.getLocalIntersectPoint() : hit.getWorldIntersectPoint();
			
			if (!handleMovingModels) {
				OsgManager::getInstance()->showPick(position);
			}
		}
	}

	void setValid(bool _valid) {
		valid = _valid;
	}

public:
	bool valid = false;

protected:
	virtual ~PickDistanceHandler() {}
};

class ImguiMainPage : public OsgImGuiHandler {
public:
    ImguiMainPage();
    ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler);
    ~ImguiMainPage();

protected:
    void drawUi();

private:
    char* cFileName;
    char* cTexturePath;
    osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
    osg::ref_ptr< CameraHandler> m_pCameraHandler = nullptr;
    std::string sFileName = "E:/projects/r3live-lab-res/textured_mesh.obj";
    const int nMaxFileNameLength = 128;

    PickDistanceHandler* picker = nullptr;
};
