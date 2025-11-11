#include <iostream>

#include <osgViewer/Viewer>
#include <osgViewer/config/SingleWindow>
#include <osgGA/TrackballManipulator>

#include "ImguiMainPage.h"

class ImGuiInitOperation : public osg::Operation {
public:
	ImGuiInitOperation() : osg::Operation("ImGuiInitOperation", false) {}

	void operator()(osg::Object* object) override {
		osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>(object);
		if (!context) {
			return;
		}

		if (!ImGui_ImplOpenGL3_Init("#version 410")) {
			std::cout << "ImGui_ImplOpenGL3_Init failed!" << std::endl;
		}
	}
};

int main() {
	osgViewer::Viewer viewer;
	viewer.apply(new osgViewer::SingleWindow(200, 200, 800, 800));
	viewer.setRealizeOperation(new ImGuiInitOperation);

	osg::ref_ptr< CameraHandler> pCameraHandler = new CameraHandler(viewer);
	viewer.addEventHandler(pCameraHandler);

	osg::Vec3 eye, up, center;
	viewer.getCamera()->getViewMatrixAsLookAt(eye, center, up);
	eye = osg::Vec3(1, 1, 0);
	center = osg::Vec3(0, 0, 0);
	up = osg::Vec3(0, 0, 1);
	viewer.getCamera()->setViewMatrixAsLookAt(eye, center, up);

	auto page = new ImguiMainPage(viewer, pCameraHandler);
	viewer.addEventHandler(page);
	while (!viewer.done()) {
		viewer.frame();
	}
	return 0;
}