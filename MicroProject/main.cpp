#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <fstream>
#include <filesystem>
#include <string>
namespace fs = std::filesystem;
const float G = 10;
const float T = 30;
class Particle
{
private:
    int mass = 1;
    float vx = 0;
    float vy = 0;
    float ax = 0;
    float ay = 0;
    static constexpr float dt = 1.0 / 16;

public:
    float r = 1;
    float x = 0;
    float y = 0;
    sf::CircleShape image = sf::CircleShape(r, 4);

public:
    void collision(Particle *other)
    {

        // закон сохранения импульса
        vx = (mass * vx + other->mass * other->vx) / (mass + other->mass);
        vy = (mass * vy + other->mass * other->vy) / (mass + other->mass);
        // BIG CRUTCH
        r *= pow((mass + other->mass) / mass, 1.0 / 3);
        image.setRadius(round(r));
        mass += other->mass;

        other->mass = 0;
        other->ax = 0;
        other->ay = 0;
        other->vx = 0;
        other->vy = 0;
        other->x = 0;
        other->y = 0;
    }
    bool check_collision(Particle &other)
    {
        return (range(other) < (r + other.r));
        // проверка на слипание
    }

    float range(Particle &other)
    {
        return pow(pow((x - other.x), 2) + pow((y - other.y), 2), 0.5);
    }

public:
    Particle() {}
    Particle(float x, float y) : x(x), y(y)
    {
        image.setPosition(x, y);
    }
    void grav(Particle *other)
    {
        float f = G * mass * other->mass / pow(std::max(range(*other), r + other->r), 3);
        ax += f * (other->x - x) / mass;
        ay += f * (other->y - y) / mass;
        other->ax -= f * (other->x - x) / other->mass;
        other->ay -= f * (other->y - y) / other->mass;
        // реализует сразу гравитацию и у себя и у того что передали
    }
    void cleara()
    {
        ax = 0;
        ay = 0;
    }
    void move()
    {
        // использовать библиотеку https://numerary.readthedocs.io/ru/latest/# что бы использовать метод рангекуты 4 точности Dormand–Prince method
        vx += ax * dt;
        vy += ay * dt;
        x += vx * dt;
        y += vy * dt;
        image.setPosition(x, y); // можно сделать подгонку под начальные параметры окна, делать не каждый шаг
    }
    void draw(sf::RenderWindow *window)
    {
        window->draw(image);
    }
    void givev(float gvx, float gvy)
    {
        vx = gvx;
        vy = gvy;
        //std::cout << vx << " " << vy <<'\n';
    }
    void print_data()
    {
        std::cout << " x: " << x << " y: " << y << " vx: " << vx << " vy: " << vy << " ax: " << ax << " ay: " << ay << '\n';
    }
    int take_mass() { return mass; }
};
std::ostream &operator<<(std::ostream &out, Particle &dirt)
{
    out << dirt.take_mass() << " ";
    return out;
}
class Solver
{
private:
    const int m0 = 1;
    //параметры окна и пределов генерации:
    const int windowx = 1900;
    const int windowy = 1000;
    const int xl = 400;
    const int xr = windowx - 400;
    const int yl = 50;
    const int yr = windowy - 50;
    const int retention_rate = 16;
    const float dt = 1.0 / 16;
    std::string path;

public:
    std::list<Particle> arr;
    int n = 1;
    sf::RenderWindow window;

public:
    Solver(int n) : n(n)
    {
        generate();
        std::cout << "Generate: n = " << n << '\n';
        path = std::string("data_simulations/Papka_n_") + std::to_string(n) + std::string("_T_") + std::to_string(T) + std::string("/");

        fs::create_directory(path);
    }
    void process()
    {
        long int s = 0; // счетчик тиков
        sf::RenderWindow window(sf::VideoMode(windowx, windowy), "SFML works!");
        std::cout << "Simulation start \n";
        sf::Clock clock;
        sf::Time previousTime = clock.getElapsedTime();
        sf::Time currentTime;
        sf::Text precText;
        precText.setPosition(sf::Vector2f(10, 0));
        precText.setFillColor(sf::Color::White);
        precText.setCharacterSize(20);
        sf::Font font;
        font.loadFromFile("font.ttf");
        precText.setFont(font);
        sf::Texture texture;
        texture.create(window.getSize().x, window.getSize().y);
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            for (auto current = arr.begin(); current != arr.end(); ++current)
            {
                current->cleara();
            }
            for (auto current = arr.begin(); current != arr.end(); ++current)
            {
                for (auto checking = std::next(current); checking != arr.end(); ++checking)
                {
                    current->grav(&(*checking));
                }
            }
            window.clear();
            for (auto current = arr.begin(); current != arr.end(); ++current)
            {
                // std::cout << i << ' ';
                // arr[i].print_data();
                current->move();
                current->draw(&window); // отрисовку можно делать не каждый шаг, так же можно разделить вычисления и отрисовку, например сохранять в текстовик координаты тел
            }                           // надо сделать переменным, что бы не тормозить когда и так не надо
            for (auto current = arr.begin(); current != arr.end(); ++current)
            {
                auto j = std::next(current);
                while (j != arr.end())
                {
                    if (current->check_collision(*j))
                    {
                        // std::cout << "BAX" << '\n';
                        current->collision(&(*j)); // удаление сделать через метод вектора, проход сделать через итераторы
                        j = arr.erase(j);
                    }
                    else
                    {
                        ++j;
                    }
                }
            }

            currentTime = clock.getElapsedTime();
            float fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds()); // the asSeconds returns a float
            precText.setString("FPS: " + std::to_string(fps));
            window.draw(precText); // flooring it will make the frame rate a rounded number
            previousTime = currentTime;
            window.display();
            if (s % retention_rate == 0)
            {
                texture.update(window);
                texture.copyToImage().saveToFile(path + std::to_string(int(s * dt)) + std::string(".png"));
            }
            ++s;
        }
        save_data(path + std::string("data.txt"));
    }

private:
    void generate()
    {
        int t = time(NULL);
        std::seed_seq seed1{t};
        std::mt19937 e1(seed1);
        std::uniform_real_distribution<> distx(xl, xr);
        std::uniform_real_distribution<> disty(yl, yr);
        for (int i = 0; i < n; ++i)
        {
            arr.push_back(Particle(distx(e1), disty(e1)));
        }
        std::cout << T << "\n";
        std::normal_distribution<> dist(0, pow(T / m0 / 1 , 0.5));
        for (auto current = arr.begin(); current != arr.end(); ++current)
        {
            current->givev(dist(e1), dist(e1));
        }
    }
    void save_data(std::string filename)
    {
        std::ofstream f;
        f.open(filename);
        for (auto current = arr.begin(); current != arr.end(); ++current)
        {
            f << *current;
        }
        f.close();
    }
};
int main()
{
    int n = 1000;
    Solver solv(n);
    solv.process();

    return 0;
}