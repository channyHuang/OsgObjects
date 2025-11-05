#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Point_set_3/IO.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

// read/write
#include <CGAL/IO/polygon_soup_io.h>
#include <CGAL/IO/read_points.h>
#include <CGAL/IO/write_points.h>
#include <CGAL/IO/PLY.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>

// outliers
#include <CGAL/remove_outliers.h>

// simplify
#include <CGAL/grid_simplify_point_set.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>
#include <CGAL/random_simplify_point_set.h>
#include <CGAL/hierarchy_simplify_point_set.h>

// smooth
#include <CGAL/jet_smooth_point_set.h>
#include <CGAL/bilateral_smooth_point_set.h>
#include <CGAL/Scale_space_reconstruction_3/Weighted_PCA_smoother.h>

// normal
#include <CGAL/jet_estimate_normals.h>
#include <CGAL/pca_estimate_normals.h>
#include <CGAL/mst_orient_normals.h>
#include <CGAL/vcm_estimate_normals.h>

// reconstruct
#include <CGAL/poisson_surface_reconstruction.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/Scale_space_surface_reconstruction_3.h>
#include <CGAL/Scale_space_reconstruction_3/Jet_smoother.h>
#include <CGAL/Scale_space_reconstruction_3/Advancing_front_mesher.h>

// cluster
#include <CGAL/cluster_point_set.h>

// aabb
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_tree.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT FT;
typedef Kernel::Point_3 Point_3;
typedef Kernel::Vector_3 Vector_3;
typedef Kernel::Sphere_3 Sphere_3;
typedef std::pair<Point_3, Vector_3> Point_with_normal;
typedef CGAL::Point_set_3<Point_3> Point_set;
typedef CGAL::Scale_space_reconstruction_3::Weighted_PCA_smoother<Kernel> Weighted_PCA_smoother;

typedef CGAL::Surface_mesh<Point_3>                                 Surface_mesh;
typedef boost::graph_traits<Surface_mesh>::vertex_descriptor      vertex_descriptor;
typedef boost::graph_traits<Surface_mesh>::face_descriptor        face_descriptor;
namespace PMP = CGAL::Polygon_mesh_processing;


typedef CGAL::AABB_face_graph_triangle_primitive<Surface_mesh> Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;
typedef boost::optional< Tree::Intersection_and_primitive_id<Kernel::Segment_3>::Type > Segment_intersection;

namespace ReconSpace {
    void outputPoints(const std::vector<Point_3>& vpoints, const std::string& filename);
    void outputPoints(const std::vector<Point_with_normal>& vpoints, const std::string& filename);
    void outputPoints(const Point_set& vpoints, const std::string& filename);

    FT recalcSpacing(const std::vector<Point_3>& points, int k);
    FT recalcSpacingWithNormal(const std::vector<Point_with_normal>& pointWithNormals, int k);

    FT recalcSpacing(const Point_set& pointset, int k);
}