#include <iostream>
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>
float G = 10;
class Particle
{
private:
    int mass = 1;
    float vx = 0;
    float vy = 0;
    float ax = 0;
    float ay = 0;

public:
    float r = 1;
    float x = 10;
    float y = 10;
    sf::CircleShape image = sf::CircleShape(10.f);

private:
    void collision(Particle &other)
    {
        // закон сохранения импульса + зсэ
    }
    void check_collision(Particle &other)
    {
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
        float f = G * mass * other->mass / pow(range(*other), 3);
        ax += f * (other->x - x) / mass;
        ay += f * (other->y - y) / mass;
        other->ax -= f * (other->x - x) / other->mass;
        other->ay -= f * (other->y - y) / other->mass;
        // реализует сразу гравитацию и у себя и у того что передали
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

int main()
{

    int n = 5;
    Particle *arr = new Particle[n];
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML works!");
    // sf::VertexArray massiv(sf::Triangles, n);
    for (int i = 0; i < n; ++i)
    {
        arr[i] = Particle(50 * i, 20);
        //arr[i].givev(1 * i, i * i);
    }
    sf::CircleShape shape(10.f);
    int FPS = 10;
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
            arr[i].move(1);
            arr[i].draw(&window); // отрисовку можно делать не каждый шаг, так же можно разделить вычисления и отрисовку, например сохранять в текстовик координаты тел
        }
        sf::sleep(sf::seconds(1.0 / FPS)); // надо сделать переменным, что бы не тормозить когда и так не надо
        window.display();
    }
    delete[] arr;
    return 0;
}