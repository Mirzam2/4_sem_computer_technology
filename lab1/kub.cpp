#include <set>
#include <gmsh.h>

int main(int argc, char **argv)
{
    gmsh::initialize();

    gmsh::model::add("kub");

    double lc = 1e-2;
    double l = 0.1;
    gmsh::model::geo::addPoint(0, 0, 0, 2 * lc, 1);
    gmsh::model::geo::addPoint(l, 0, 0, lc, 2);
    gmsh::model::geo::addPoint(l, l, 0, lc, 3);
    gmsh::model::geo::addPoint(0, l, 0, lc, 4);
    gmsh::model::geo::addPoint(0, 0, l, lc, 5);
    gmsh::model::geo::addPoint(l, 0, l, lc, 6);
    gmsh::model::geo::addPoint(l, l, l, lc / 4, 7);
    gmsh::model::geo::addPoint(0, l, l, lc, 8);

    // нижняя крышка
    gmsh::model::geo::addLine(1, 2, 1);
    gmsh::model::geo::addLine(2, 3, 2);
    gmsh::model::geo::addLine(3, 4, 3);
    gmsh::model::geo::addLine(4, 1, 4);

    gmsh::model::geo::addCurveLoop({4, 1, 2, 3}, 1);
    gmsh::model::geo::addPlaneSurface({1}, 1);
    // верхняя крышка
    gmsh::model::geo::addLine(5, 6, 5);
    gmsh::model::geo::addLine(6, 7, 6);
    gmsh::model::geo::addLine(7, 8, 7);
    gmsh::model::geo::addLine(8, 5, 8);

    gmsh::model::geo::addCurveLoop({5, 6, 7, 8}, 2);
    gmsh::model::geo::addPlaneSurface({2}, 2);

    // боковые ребра
    for (int i = 1; i < 5; i++)
    {
        gmsh::model::geo::addLine(i, i + 4, i + 8);
    }
    for (int i = 0; i < 4; i++)
    {
        int t = 0;
        if (i == 3)
        {
            t = 4;
        }
        gmsh::model::geo::addCurveLoop({1 + i, 10 + i - t, -(5 + i), -(9 + i)}, 3 + i);
        gmsh::model::geo::addPlaneSurface({3 + i}, 3 + i);
    }

    gmsh::model::geo::addSurfaceLoop({1, 2, 3, 4, 5, 6}, 1);
    gmsh::model::geo::addVolume({1});

    gmsh::model::geo::synchronize();

    gmsh::model::mesh::generate(3);

    gmsh::write("kub.msh");

    std::set<std::string> args(argv, argv + argc);
    if (!args.count("-nopopup"))
        gmsh::fltk::run();

    gmsh::finalize();

    return 0;
}
