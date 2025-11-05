#include "commonCGAL.h"

namespace ReconSpace {
    void outputPoints(const std::vector<Point_3>& vpoints, const std::string& filename) {
        CGAL::IO::write_points(filename, vpoints);
    }

    void outputPoints(const std::vector<Point_with_normal>& vpoints, const std::string& filename) {
        CGAL::IO::write_points(filename, vpoints,
            CGAL::parameters::point_map(CGAL::First_of_pair_property_map<Point_with_normal>())
            .normal_map(CGAL::Second_of_pair_property_map<Point_with_normal>()));
    }

    void outputPoints(const Point_set& vpoints, const std::string& filename) {
        CGAL::IO::write_PLY(filename, vpoints, CGAL::parameters::use_binary_mode(false));
    }

    FT recalcSpacing(const std::vector<Point_3>& points, int k) {
        return CGAL::compute_average_spacing<CGAL::Parallel_if_available_tag>(points, k);
    }

    FT recalcSpacingWithNormal(const std::vector<Point_with_normal>& pointWithNormals, int k) {
        return CGAL::compute_average_spacing<CGAL::Parallel_if_available_tag>(pointWithNormals, k,
            CGAL::parameters::point_map(CGAL::First_of_pair_property_map<Point_with_normal>()));
    }

    FT recalcSpacing(const Point_set& pointset, int k) {
        return CGAL::compute_average_spacing<CGAL::Parallel_if_available_tag>(pointset, k);
    }
}