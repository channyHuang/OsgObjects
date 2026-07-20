#pragma once

#include <iostream>

#include <osgViewer/Viewer>

#include "commonOsg/osgCameraHandler.h"
#include "commonFunc/signalSlots.h"

class CameraHandlerTerrain : public CameraHandler {
public:
	CameraHandlerTerrain();
	CameraHandlerTerrain(osgViewer::Viewer& viewer);

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	virtual void reset();
	virtual void back2WorldCenter();
	
public:
	bool m_bBack2WorldCenter = false;
	int m_nAxis = 0;
	float m_fStepScale = .5f;
	SignalSlot::Signal<void(const osg::Vec3&)> sigPick;
	bool m_bEditActivated = false;

protected:
	virtual ~CameraHandlerTerrain();
};