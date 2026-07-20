#pragma once

#include <iostream>

#include "CameraHandlerFix.h"

CameraHandlerFix::CameraHandlerFix() {}

CameraHandlerFix::CameraHandlerFix(osgViewer::Viewer& viewer) { 
	m_pViewer = &viewer;
	m_vCenter = osg::Vec3(0.f, 0.f, 0.f);
	m_vEye = osg::Vec3(100.f, 100.f, 100.f);
	m_vUp = osg::Vec3(-1.f, 1.f, -1.f);
	m_fViewDistance = (m_vEye - m_vCenter).length();
	m_pViewer->getCamera()->setViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp);
}

CameraHandlerFix::~CameraHandlerFix() {}

bool CameraHandlerFix::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
	switch (ea.getEventType()) {
	case(osgGA::GUIEventAdapter::KEYUP):
	{
		// move camera (up/down/left/right/front/back)
		m_pViewer->getCamera()->getViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp, m_fViewDistance);

		osg::Vec3 vLookDir = (m_vCenter - m_vEye);
		vLookDir.normalize();
		osg::Vec3 vLookLeft = m_vUp ^ vLookDir;
		vLookLeft.normalize();

		vLookLeft = vLookLeft * m_fStepScale;
		vLookDir = vLookDir * m_fStepScale;

		switch (ea.getKey()) {
		case osgGA::GUIEventAdapter::KEY_Q:
			m_vEye += m_vUp * m_fStepScale;
			// m_vCenter += m_vUp * m_fStepScale;
			break;
		case osgGA::GUIEventAdapter::KEY_E:
			m_vEye -= m_vUp * m_fStepScale;
			// m_vCenter -= m_vUp * m_fStepScale;
			break;
		case osgGA::GUIEventAdapter::KEY_W:
			m_vEye += vLookDir;
			// m_vCenter += vLookDir;
			break;
		case osgGA::GUIEventAdapter::KEY_S:
			m_vEye -= vLookDir;
			// m_vCenter -= vLookDir;
			break;
		case osgGA::GUIEventAdapter::KEY_A:
			m_vEye += vLookLeft;
			// m_vCenter += vLookLeft;
			break;
		case osgGA::GUIEventAdapter::KEY_D:
			m_vEye -= vLookLeft;
			// m_vCenter -= vLookLeft;
			break;
		default:
			return false;
		}

		m_fViewDistance = (m_vEye - m_vCenter).length();
		m_pViewer->getCamera()->setViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp);
	}
	break;
	case (osgGA::GUIEventAdapter::SCROLL):
	{
		m_pViewer->getCamera()->getViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp, m_fViewDistance);
		if (m_fViewDistance < 1) return false;

		osg::Vec3 vLookDir = (m_vCenter - m_vEye);
		vLookDir.normalize();
		
		switch (ea.getScrollingMotion())
		{
		case(osgGA::GUIEventAdapter::SCROLL_DOWN):
			m_vEye += vLookDir;
			break;
		case(osgGA::GUIEventAdapter::SCROLL_UP):
			m_vEye -= vLookDir;
			break;
		default:
			return false;
		}

		m_fViewDistance = (m_vEye - m_vCenter).length();
		m_pViewer->getCamera()->setViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp);
	}
	break;
	case (osgGA::GUIEventAdapter::DRAG): {
		if (!m_bEditActivated) {
			if (m_fPreX < 0 || m_fPreY < 0) {
				m_fPreX = ea.getX();
				m_fPreY = ea.getY();
				return false;
			}
			// original axis
			m_pViewer->getCamera()->getViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp, m_fViewDistance);
			osg::Vec3 vLookDir = (m_vCenter - m_vEye);
			vLookDir.normalize();
			osg::Vec3 vLookLeft = m_vUp ^ (m_vCenter - m_vEye);
			vLookLeft.normalize();
			
			// rotate
			osg::Matrix matRotUp = osg::Matrix::rotate(osg::inDegrees((ea.getX() - m_fPreX) * m_fStepScale), m_vUp);
			osg::Matrix matRotLeft = osg::Matrix::rotate(osg::inDegrees((ea.getY() - m_fPreY) * m_fStepScale), vLookLeft);

			vLookDir = matRotUp * vLookDir;
			vLookDir.normalize();
			m_vEye = m_vCenter - (vLookDir * m_fViewDistance);
			vLookLeft = m_vUp ^ vLookDir;
			
			vLookDir = matRotLeft * vLookDir;
			vLookDir.normalize();
			m_vEye = m_vCenter - (vLookDir * m_fViewDistance);
			m_vUp = vLookDir ^ vLookLeft;
			m_vUp.normalize();

			// update camera
			m_fViewDistance = (m_vEye - m_vCenter).length();
			m_pViewer->getCamera()->setViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp);
			m_fPreX = ea.getX();
			m_fPreY = ea.getY();
		} else {
			osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
			if (!pView) return false;

			osgUtil::LineSegmentIntersector* pIntersector =
				new osgUtil::LineSegmentIntersector(
					osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
			osgUtil::IntersectionVisitor visitor(pIntersector);
			pView->getCamera()->accept(visitor);

			if (pIntersector->containsIntersections()) {
				auto hit = pIntersector->getFirstIntersection();
				osg::Vec3 worldPos = hit.getWorldIntersectPoint(); // 命中点的世界坐标
				// osg::NodePath nodePath = hit.nodePath;             // 命中节点路径
				// sigPick(worldPos);
			}
		}
	}
	break;
	case (osgGA::GUIEventAdapter::RELEASE):	{
		m_fPreX = -1;
		m_fPreY = -1;
	}
	break;
	case (osgGA::GUIEventAdapter::PUSH): {
        if (m_bEditActivated) {
			osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
			if (!pView) return false;

			osgUtil::LineSegmentIntersector* pIntersector =
				new osgUtil::LineSegmentIntersector(
					osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
			osgUtil::IntersectionVisitor visitor(pIntersector);
			pView->getCamera()->accept(visitor);

			if (pIntersector->containsIntersections()) {
				auto hit = pIntersector->getFirstIntersection();
				osg::Vec3 worldPos = hit.getWorldIntersectPoint(); // 命中点的世界坐标
				// osg::NodePath nodePath = hit.nodePath;             // 命中节点路径
				// sigPick(worldPos);
			}
		}
    }
	break;
	default:
		return false;
	}

	return false;
}

void CameraHandlerFix::reset() {
	m_vCenter = osg::Vec3(0.f, 0.f, 0.f);
	m_vEye = osg::Vec3(100.f, 100.f, 100.f);
	m_vUp = osg::Vec3(-1.f, 1.f, -1.f);
	m_fViewDistance = 10.f;
	m_pViewer->getCamera()->setViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp);
}

void CameraHandlerFix::back2WorldCenter() {
	m_pViewer->getCamera()->getViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp, m_fViewDistance);

	m_vCenter = osg::Vec3(0.f, 0.f, 0.f);
	m_fViewDistance = (m_vEye - m_vCenter).length();
	m_pViewer->getCamera()->setViewMatrixAsLookAt(m_vEye, m_vCenter, m_vUp);
}
