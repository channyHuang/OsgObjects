#pragma once

#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

class OsgImGuiHandler : public osgGA::GUIEventHandler
{
public:
	OsgImGuiHandler();

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

protected:
	virtual void drawUi() = 0;

private:
	void init();
	void setCameraCallbacks(osg::Camera* camera);
	void newFrame(osg::RenderInfo& renderInfo);
	void render(osg::RenderInfo& renderInfo);

private:
	struct ImGuiNewFrameCallback;
	struct ImGuiRenderCallback;

	double time_;
	bool mousePressed_[3];
	float mouseWheel_;
	bool initialized_;

};

