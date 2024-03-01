#pragma once

#include "osgManager.h"

class PickHandler : public osgGA::GUIEventHandler {
public:
	PickHandler() {}

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
			std::cout << "click " << position.x() << " " << position.y() << " " << position.z() << std::endl;
			if (!handleMovingModels) {
				//TerrainModification::getInstance()->modify((TerrainModifyType)(OsgManager::getInstance()->modifyType), Vector3(position.x(), position.y(), position.z()));

				
			}
		}
	}

protected:
	virtual ~PickHandler() {}

	bool valid = false;
};

class CameraHandler : public osgGA::GUIEventHandler {
public:
	CameraHandler() {}
	CameraHandler(osgViewer::Viewer& viewer) { 
		pviewer = &viewer;
		pviewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
	}
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
		switch (ea.getEventType())
		{
		case(osgGA::GUIEventAdapter::KEYUP):
		{
			pviewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);

			osg::Vec3 dir = (center - eye);
			dir.normalize();
			osg::Vec3 right = up ^ dir;
			right.normalize();

			switch (ea.getKey()) {
			case osgGA::GUIEventAdapter::KEY_Q:
				eye += up;
				center += up;
				break;
			case osgGA::GUIEventAdapter::KEY_E:
				eye -= up;
				center -= up;
				break;
			case osgGA::GUIEventAdapter::KEY_W:
				eye += dir;
				center += dir;
				break;
			case osgGA::GUIEventAdapter::KEY_S:
				eye -= dir;
				center -= dir;
				break;
			case osgGA::GUIEventAdapter::KEY_A:
				eye -= right;
				center -= right;
				break;
			case osgGA::GUIEventAdapter::KEY_D:
				eye += right;
				center += right;
				break;
			default:
				return false;
			}

			pviewer->getCamera()->setViewMatrixAsLookAt(eye, center, up);
		}
		break;
		case (osgGA::GUIEventAdapter::SCROLL):
		{
			osg::Vec3 dir = (center - eye);
			dir.normalize();

			switch (ea.getScrollingMotion())
			{
			case(osgGA::GUIEventAdapter::SCROLL_DOWN):
				eye += dir;
				center += dir;
				break;
			case(osgGA::GUIEventAdapter::SCROLL_UP):
				eye -= dir;
				center -= dir;
				break;
			default:
				return false;
			}

			pviewer->getCamera()->setViewMatrixAsLookAt(eye, center, up);
		}
		break;
		case (osgGA::GUIEventAdapter::DRAG):
		{
			if (preX < 0 || preY < 0) {
				preX = ea.getX();
				preY = ea.getY();
				return false;
			}

			osg::Vec3 direye = (center - eye);

			if (bRotateByAxis) {
				osg::Matrix rot;
				switch (axis) {
				case 0:
					rot = osg::Matrix::rotate(osg::inDegrees(ea.getX() - preX), osg::Vec3(1, 0, 0));
					break;
				case 1:
					rot = osg::Matrix::rotate(osg::inDegrees(ea.getX() - preX), osg::Vec3(0, 1, 0));
					break;
				case 2:
					rot = osg::Matrix::rotate(osg::inDegrees(ea.getX() - preX), osg::Vec3(0, 0, 1));
					break;
				default:
					break;
				}
				up = rot * up;
				eye = center - rot * direye;
			}
			else {
				osg::Matrix rotup = osg::Matrix::rotate(osg::inDegrees(ea.getX() - preX), up);

				center = eye + rotup * direye;
				osg::Vec3 dirz = up ^ (center - eye);

				osg::Matrix rotz = osg::Matrix::rotate(osg::inDegrees(ea.getY() - preY), dirz);
				up = rotz * up;
				direye = -up ^ dirz;
				center = eye + direye;
			}
			
			pviewer->getCamera()->setViewMatrixAsLookAt(eye, center, up);
			preX = ea.getX();
			preY = ea.getY();
		}
		break;
		case (osgGA::GUIEventAdapter::RELEASE):
		{
			preX = -1;
			preY = -1;
		}
		break;
		default:
			return false;
		}

		return false;
	}

	bool bRotateByAxis = false;
	int axis = 0;
protected:
	virtual ~CameraHandler() {}
	osg::ref_ptr<osgViewer::Viewer> pviewer;
	float theta = 0, phi = 0;
	osg::Vec3 eye, up, center;
	float preX = -1, preY = -1;
	osg::Matrix axisRot = osg::Matrix::identity();
};