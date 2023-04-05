#include <iostream>
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <fstream>
float G = 10;
float T = 10;
class Particle
{
private:
    int mass = 1;
    float vx = 0;
    float vy = 0;
    float ax = 0;
    float ay = 0;

public:
    float r = 0.1;
    float x = 10;
    float y = 10;
    sf::CircleShape image = sf::CircleShape(1.f);

public:
    void collision(Particle *other)
    {

        // закон сохранения импульса
        vx = (mass * vx + other->mass * other->vx) / (mass + other->mass);
        vy = (mass * vy + other->mass * other->vy) / (mass + other->mass);
        // BIG CRUTCH
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
        return (range(other) < r + other.r);
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
        if (mass * other->mass == 0)
        {
            return; // КОСТЫЛЬ
        }
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
    void move(float dt)
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
    }
    void print_data()
    {
        std::cout << " x: " << x << " y: " << y << " vx: " << vx << " vy: " << vy << " ax: " << ax << " ay: " << ay << '\n';
    }
};
// class Solver
// {
// private:
//     const float G = 10;
//     const float T = 10;
//     int n = 4;

// public:
//     Particle *arr;

// public:
//     Solver(int n) : n(n)
//     {

//     }

// private:
// };
int main()
{

    int n = 400;
    int m0 = 1;
    std::vector<Particle> arr;
    sf::RenderWindow window(sf::VideoMode(1600, 1000), "SFML works!");
    // sf::VertexArray massiv(sf::Triangles, n);
    for (int i = 0; i < pow(n, 0.5); ++i)
    {
        for (int j = 0; j < pow(n, 0.5); ++j)
        {
            arr.push_back(Particle(10 * i + 200, 10 * j + 200));
        }
    }
    // int t = time(NULL);
    std::seed_seq seed2{0};
    std::mt19937 e2(seed2);
    std::normal_distribution<> dist(0, pow(T / m0, 0.5));
    for (int i = 0; i < n; ++i)
    {
        arr[i].givev(dist(e2), dist(e2));
    }
    // sf::Clock clock;
    // sf::Time previousTime = clock.getElapsedTime();
    // sf::Time currentTime;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        for (int i = 0; i < n; ++i)
        {
            arr[i].cleara();
        }
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                arr[i].grav(&arr[j]);
            }
        }
        window.clear();
        for (int i = 0; i < n; ++i)
        {
            // std::cout << i << ' ';
            // arr[i].print_data();
            arr[i].move(0.01);
            arr[i].draw(&window); // отрисовку можно делать не каждый шаг, так же можно разделить вычисления и отрисовку, например сохранять в текстовик координаты тел
        }                         // надо сделать переменным, что бы не тормозить когда и так не надо
        window.display();
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                if (arr[i].check_collision(arr[j]))
                {
                    std::cout << "BAX" << '\n';
                    arr[i].collision(&arr[j]);
                }
            }
        }

        // currentTime = clock.getElapsedTime();
        // float fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds()); // the asSeconds returns a float
        // std::cout << "fps =" << floor(fps) << std::endl;                         // flooring it will make the frame rate a rounded number
        // previousTime = currentTime;
    }
    return 0;
}