
#include <iostream>

#include <osgViewer/Viewer>
#include <osgViewer/config/SingleWindow>
#include <osgGA/TrackballManipulator>

#include "ImguiMainPage.h"
#include "CameraHandlerFix.h"

class ImGuiInitOperation : public osg::Operation {
public:
	ImGuiInitOperation() : osg::Operation("ImGuiInitOperation", false) {}

	void operator()(osg::Object* object) override {
		osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>(object);
		if (!context) {
			return;
		}

		if (!ImGui_ImplOpenGL3_Init()) {
			std::cout << "ImGui_ImplOpenGL3_Init failed!" << std::endl;
		}
	}
};

int main() {
	osgViewer::Viewer viewer;
	viewer.apply(new osgViewer::SingleWindow(200, 200, 800, 800));
	viewer.setRealizeOperation(new ImGuiInitOperation);

	osg::ref_ptr< CameraHandlerFix> pCameraHandler = new CameraHandlerFix(viewer);
	viewer.addEventHandler(pCameraHandler);

	auto page = new ImguiMainPage(viewer, pCameraHandler);
	viewer.addEventHandler(page);

	while (!viewer.done()) {
		viewer.frame();
	}
	return 0;
}