#include <set>
#include <gmsh.h>

int main(int argc, char **argv)
{
    gmsh::initialize();

    gmsh::model::add("circle");

    double lc = 1e-2;
    double r = 0.1;
    auto center = gmsh::model::geo::addPoint(0, 0, 0, lc);
    gmsh::model::geo::addPoint(r, 0, 0, lc, 2);
    gmsh::model::geo::addPoint(0, r, 0, lc, 3);
    gmsh::model::geo::addPoint(-r, 0, 0, lc, 4);
    gmsh::model::geo::addPoint(0, -r, 0, lc, 5);
    gmsh::model::geo::addCircleArc(2, center, 3, 1);
    gmsh::model::geo::addCircleArc(3, center, 4, 2);
    gmsh::model::geo::addCircleArc(4, center, 5, 3);
    gmsh::model::geo::addCircleArc(5, center, 2, 4);
    gmsh::model::geo::addCurveLoop({1, 2,3,4}, 1);
    gmsh::model::geo::addPlaneSurface({1}, 1);
    gmsh::vectorpair ov1;
    gmsh::model::geo::extrude({{2, 1}}, 0, 0, 0.1, ov1, {}, {}, true);
    gmsh::model::geo::synchronize();

    gmsh::model::mesh::generate(3);

    gmsh::write("circle.msh");

    std::set<std::string> args(argv, argv + argc);
    if (!args.count("-nopopup"))
        gmsh::fltk::run();

    gmsh::finalize();

    return 0;
}
