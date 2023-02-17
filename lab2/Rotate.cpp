#include <iostream>
#include <cmath>
#include <vector>

#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkTetra.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>

#include <gmsh.h>

using namespace std;

// Класс расчётной точки
class CalcNode
{
    // Класс сетки будет friend-ом точки
    friend class CalcMesh;

protected:
    // Координаты
    double x;
    double y;
    double z;
    // Некая величина, в попугаях
    double smth;
    // Скорость
    double vx;
    double vy;
    double vz;

public:
    // Конструктор по умолчанию
    CalcNode() : x(0.0), y(0.0), z(0.0), smth(0.0), vx(0.0), vy(0.0), vz(0.0)
    {
    }

    // Конструктор с указанием всех параметров
    CalcNode(double x, double y, double z, double smth, double vx, double vy, double vz)
        : x(x), y(y), z(z), smth(smth), vx(vx), vy(vy), vz(vz)
    {
    }

    // Метод отвечает за перемещение точки
    // Движемся время tau из текущего положения с текущей скоростью
    void move(double tau)
    {
        x += vx * tau;
        y += vy * tau;
        z += vz * tau;
    }
    void collapce(double g, double cx = 0, double cy = 0, double cz = 0)
    {
        vx = -g * (x - cx);
        vy = -g * (y - cy);
        vz = g * (pow(x - cx, 2) + pow(y - cy, 2));
    }
};

// Класс элемента сетки
class Element
{
    // Класс сетки будет friend-ом и элемента тоже
    // (и вообще будет нагло считать его просто структурой)
    friend class CalcMesh;

protected:
    // Индексы узлов, образующих этот элемент сетки
    unsigned long nodesIds[4];
};

// Класс расчётной сетки
class CalcMesh
{
protected:
    // 3D-сетка из расчётных точек
    vector<CalcNode> nodes;
    vector<Element> elements;

public:
    // Конструктор сетки из заданного stl-файла
    CalcMesh(const std::vector<double> &nodesCoords, const std::vector<std::size_t> &tetrsPoints)
    {

        // Пройдём по узлам в модели gmsh
        nodes.resize(nodesCoords.size() / 3);
        for (unsigned int i = 0; i < nodesCoords.size() / 3; i++)
        {
            // Координаты заберём из gmsh
            double pointX = nodesCoords[i * 3];
            double pointY = nodesCoords[i * 3 + 1];
            double pointZ = nodesCoords[i * 3 + 2];
            // Модельная скалярная величина распределена как-то вот так
            double smth = sin(pointX) * (pow(pointX, 2) + pow(pointY, 2) + pow(pointZ, 2));
            nodes[i] = CalcNode(pointX, pointY, pointZ, smth, 0.0, 0.0, 0.0);
        }

        // Пройдём по элементам в модели gmsh
        elements.resize(tetrsPoints.size() / 4);
        for (unsigned int i = 0; i < tetrsPoints.size() / 4; i++)
        {
            elements[i].nodesIds[0] = tetrsPoints[i * 4] - 1;
            elements[i].nodesIds[1] = tetrsPoints[i * 4 + 1] - 1;
            elements[i].nodesIds[2] = tetrsPoints[i * 4 + 2] - 1;
            elements[i].nodesIds[3] = tetrsPoints[i * 4 + 3] - 1;
        }
    }

    // Метод отвечает за выполнение для всей сетки шага по времени величиной tau
    void doTimeStep(double tau)
    {
        // По сути метод просто двигает все точки
        for (unsigned int i = 0; i < nodes.size(); i++)
        {
            nodes[i].move(tau);
        }
    }
    void setRotationOptions(double omega, double ax, double ay, double az, double px = 0, double py = 0, double pz = 0)
    {
        // задает скорости нужные для вращения
        double a = sqrt(pow(ax, 2) + pow(ay, 2) + pow(az, 2));
        ax = ax / a * omega;
        ay = ay / a * omega;
        az = az / a * omega;
        for (unsigned int i = 0; i < nodes.size(); i++)
        {
            nodes[i].vx = ay * (nodes[i].z - pz) - az * (nodes[i].y - py);
            nodes[i].vy = az * (nodes[i].x - px) - ax * (nodes[i].z - pz);
            nodes[i].vz = ax * (nodes[i].y - py) - ay * (nodes[i].x - px);
        }
    }
    void Collapce(double g)
    {
        for (unsigned int i = 0; i < nodes.size(); i++)
        {
            nodes[i].collapce(g, -5.85283, 0.19466, 1.4);
        }
    }
    // Метод отвечает за запись текущего состояния сетки в снапшот в формате VTK
    void snapshot(unsigned int snap_number)
    {
        // Сетка в терминах VTK
        vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
        // Точки сетки в терминах VTK
        vtkSmartPointer<vtkPoints> dumpPoints = vtkSmartPointer<vtkPoints>::New();

        // Скалярное поле на точках сетки
        auto smth = vtkSmartPointer<vtkDoubleArray>::New();
        smth->SetName("smth");

        // Векторное поле на точках сетки
        auto vel = vtkSmartPointer<vtkDoubleArray>::New();
        vel->SetName("velocity");
        vel->SetNumberOfComponents(3);

        // Обходим все точки нашей расчётной сетки
        for (unsigned int i = 0; i < nodes.size(); i++)
        {
            // Вставляем новую точку в сетку VTK-снапшота
            dumpPoints->InsertNextPoint(nodes[i].x, nodes[i].y, nodes[i].z);

            // Добавляем значение векторного поля в этой точке
            double _vel[3] = {nodes[i].vx, nodes[i].vy, nodes[i].vz};
            vel->InsertNextTuple(_vel);

            // И значение скалярного поля тоже
            smth->InsertNextValue(nodes[i].smth);
        }

        // Грузим точки в сетку
        unstructuredGrid->SetPoints(dumpPoints);

        // Присоединяем векторное и скалярное поля к точкам
        unstructuredGrid->GetPointData()->AddArray(vel);
        unstructuredGrid->GetPointData()->AddArray(smth);

        // А теперь пишем, как наши точки объединены в тетраэдры
        for (unsigned int i = 0; i < elements.size(); i++)
        {
            auto tetra = vtkSmartPointer<vtkTetra>::New();
            tetra->GetPointIds()->SetId(0, elements[i].nodesIds[0]);
            tetra->GetPointIds()->SetId(1, elements[i].nodesIds[1]);
            tetra->GetPointIds()->SetId(2, elements[i].nodesIds[2]);
            tetra->GetPointIds()->SetId(3, elements[i].nodesIds[3]);
            unstructuredGrid->InsertNextCell(tetra->GetCellType(), tetra->GetPointIds());
        }

        // Создаём снапшот в файле с заданным именем
        string fileName = "rotate" + std::to_string(snap_number) + ".vtu";
        vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
        writer->SetFileName(fileName.c_str());
        writer->SetInputData(unstructuredGrid);
        writer->Write();
    }
};

int main()
{
    std::cout << "Lets go\n";
    // Шаг точек по пространству
    double h = 4.0;
    // Шаг по времени
    double tau = 0.01;

    const unsigned int GMSH_TETR_CODE = 4;

    // Теперь придётся немного упороться:
    // (а) построением сетки средствами gmsh,
    // (б) извлечением данных этой сетки в свой код.
    gmsh::initialize();
    gmsh::model::add("suriken");

    // Считаем STL
    try
    {
        gmsh::merge("suriken.stl");
    }
    catch (...)
    {
        std::cout << "Nope";
        gmsh::logger::write("Could not load STL mesh: bye!");
        gmsh::finalize();
        return -1;
    }
    std::cout << "Konect ok\n";
    // Восстановим геометрию
    double angle = 10;
    bool forceParametrizablePatches = false;
    bool includeBoundary = true;
    double curveAngle = 180;
    gmsh::model::mesh::classifySurfaces(angle * M_PI / 180., includeBoundary, forceParametrizablePatches);
    gmsh::model::mesh::createGeometry();

    // Зададим объём по считанной поверхности
    std::vector<std::pair<int, int>> s;
    gmsh::model::getEntities(s, 2);
    std::vector<int> sl;
    for (auto surf : s)
        sl.push_back(surf.second);
    int l = gmsh::model::geo::addSurfaceLoop(sl);
    gmsh::model::geo::addVolume({l});

    gmsh::model::geo::synchronize();
    std::cout << "Merge ok \n";
    // Зададим мелкость желаемой сетки
    int f = gmsh::model::mesh::field::add("MathEval");
    gmsh::model::mesh::field::setString(f, "F", "4");
    gmsh::model::mesh::field::setAsBackgroundMesh(f);

    // Построим сетку
    gmsh::model::mesh::generate(3);

    // Теперь извлечём из gmsh данные об узлах сетки
    std::vector<double> nodesCoord;
    std::vector<std::size_t> nodeTags;
    std::vector<double> parametricCoord;
    gmsh::model::mesh::getNodes(nodeTags, nodesCoord, parametricCoord);

    // И данные об элементах сетки тоже извлечём, нам среди них нужны только тетраэдры, которыми залит объём
    std::vector<std::size_t> *tetrsNodesTags = nullptr;
    std::vector<int> elementTypes;
    std::vector<std::vector<std::size_t>> elementTags;
    std::vector<std::vector<std::size_t>> elementNodeTags;
    gmsh::model::mesh::getElements(elementTypes, elementTags, elementNodeTags);
    for (unsigned int i = 0; i < elementTypes.size(); i++)
    {
        if (elementTypes[i] != GMSH_TETR_CODE)
            continue;
        tetrsNodesTags = &elementNodeTags[i];
    }

    if (tetrsNodesTags == nullptr)
    {
        cout << "Can not find tetra data. Exiting." << endl;
        gmsh::finalize();
        return -2;
    }

    cout << "The model has " << nodeTags.size() << " nodes and " << tetrsNodesTags->size() / 4 << " tetrs." << endl;

    // На всякий случай проверим, что номера узлов идут подряд и без пробелов
    for (int i = 0; i < nodeTags.size(); ++i)
    {
        // Индексация в gmsh начинается с 1, а не с нуля. Ну штош, значит так.
        assert(i == nodeTags[i] - 1);
    }
    // И ещё проверим, что в тетраэдрах что-то похожее на правду лежит.
    assert(tetrsNodesTags->size() % 4 == 0);
    std::cout << "Checking ok \n";
    // TODO: неплохо бы полноценно данные сетки проверять, да

    CalcMesh mesh(nodesCoord, *tetrsNodesTags);
    std::cout << "AAAAAAAAAAA\n";

    mesh.snapshot(0);
    for (unsigned int step = 1; step < 100; step++)
    {
        mesh.setRotationOptions(1, 0, 0, 1, -5.85283, 0.19466, 0);
        mesh.doTimeStep(tau);
        mesh.snapshot(step);
    }
    gmsh::finalize();
    std::cout << "End";
    return 0;
}
