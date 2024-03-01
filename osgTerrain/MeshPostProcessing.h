#pragma once

#include <string>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Point_set_3/IO.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

#include "xatlas/xatlas.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT FT;
typedef Kernel::Point_3 Point_3;
typedef boost::graph_traits<CGAL::Surface_mesh<Point_3>>::vertex_descriptor      vertex_descriptor;
typedef boost::graph_traits<CGAL::Surface_mesh<Point_3>>::face_descriptor        face_descriptor;

class MeshPostProcessing
{
public:
	static MeshPostProcessing* getInstance() {
		if (instance == nullptr) {
			instance = new MeshPostProcessing();
		}
		return instance;
	}
	~MeshPostProcessing() {}

	bool unwrap();
	bool unwrap(const std::string& sFileName, std::vector<std::pair<float, float>>& uvs);

public:
	std::string sFileName = "";
	bool bProcessingFinish = false;
	std::vector<uint8_t> chartsTextureData;
	int uvWidth = 0, uvHeight = 0;

private:
	MeshPostProcessing() {}

private:
	CGAL::Surface_mesh<Point_3> output_mesh;
	std::vector<std::uint32_t> indices;
	xatlas::Atlas* atlas;

	static MeshPostProcessing* instance;
};

