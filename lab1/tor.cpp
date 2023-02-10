#include <set>
#include <gmsh.h>
#include <iostream>
int make_tor(float r, float R)
{
    int nc = 3; // number of centers
    int np = 9; // number of points in one mini circle
    double lc = 1e-2;
    int centers[nc];
    int points[nc][np];
    int arcs[nc][np];
    std::vector<int> surf;
    surf.assign(nc * np, 0);
    for (int i = 0; i < nc; ++i)
    {
        float a = 2 * i * M_PI / nc;
        centers[i] = gmsh::model::geo::addPoint(R * cos(a), R * sin(a), 0, lc);

        for (int j = 0; j < np; ++j)
        {
            float b = 2 * M_PI * j / np;
            points[i][j] = gmsh::model::geo::addPoint((R + r * cos(b)) * cos(a), (R + r * cos(b)) * sin(a), r * sin(b));
        }

        for (int j = 0; j < np - 1; ++j)
        {
            arcs[i][j] = gmsh::model::geo::addCircleArc(points[i][j], centers[i], points[i][j + 1]);
        }
        arcs[i][np - 1] = gmsh::model::geo::addCircleArc(points[i][np - 1], centers[i], points[i][0]);
        for (int j = 0; j < np; ++j)
        {
            std::vector<std::pair<int, int>> ov;
            gmsh::model::geo::revolve({{1, arcs[i][j]}}, 0, 0, 0, 0, 0, 1, 2 * M_PI / nc, ov);
            surf[i * np + j] = ov[1].second;
        }
    }
    return gmsh::model::geo::addSurfaceLoop(surf);
}
int main(int argc, char **argv)
{
    gmsh::initialize();

    gmsh::model::add("tor");

    float r0 = 0.4, r1 = 0.2, r2 = 0.1;
    auto t1 = make_tor(r1, r0);
    auto t2 = make_tor(r2, r0);

    gmsh::model::geo::addVolume({t1,-t2});
    gmsh::model::geo::synchronize();

    gmsh::model::mesh::generate(3);

    gmsh::write("tor.msh");

    std::set<std::string> args(argv, argv + argc);
    if (!args.count("-nopopup"))
        gmsh::fltk::run();

    gmsh::finalize();

    return 0;
}
