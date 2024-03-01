#include <iostream>

#include <osgViewer/Viewer>
#include <osgViewer/config/SingleWindow>
#include <osgGA/TrackballManipulator>

#include "ImguiMainPage.h"
#include "osgPickHandler.h"

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
	viewer.setCameraManipulator(new osgGA::TrackballManipulator());
	viewer.addEventHandler(new CameraHandler(viewer));

	auto page = new ImguiMainPage(viewer);
	viewer.addEventHandler(page);

	viewer.getCamera()->getGraphicsContext()->getState()->resetVertexAttributeAlias(false);
	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);

	return viewer.run();
}