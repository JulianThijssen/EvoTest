#include "Renderer.h"
#include "PolyLine.h"
#include "Recorder.h"

#include <GDT/Window.h>

#include <GDT/Framebuffer.h>
#include <GDT/Texture.h>
#include <GDT/Vector2f.h>
#include <GDT/Vector3f.h>
#include <GDT/Math.h>
#include <GDT/OpenGL.h>

#include <random>
#include <vector>
#include <numeric>
#include <set>
#include <chrono>
#include <unordered_set>

#include <iomanip>
#include <iostream>
#include <sstream>

#include <box2d/box2d.h>

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution;

Vector2f randomDirection()
{
    float x = distribution(generator) * 2 - 1;
    float y = distribution(generator) * 2 - 1;
    Vector2f v(x, y);
    return normalize(v);
}

Vector2f randomPosition(Vector2f range)
{
    float x = distribution(generator) * range.x - range.x / 2;
    float y = distribution(generator) * range.y - range.y / 2;
    
    return Vector2f(x, y);
}

float randomAngle()
{
    return distribution(generator) * Math::TWO_PI;
}

Vector2f angleToVec(float angle)
{
    return Vector2f(cos(angle), sin(angle));
}

class Cell
{
public:
    Vector2f pos;
    Vector2f vel;
    float radius;
    b2Body* body;
};


struct CollisionPair
{
    CollisionPair() :
        i(0), j(0)
    { }

    CollisionPair(int first, int second)
    {
        if (first <= second)
        {
            i = first;
            j = second;
        }
        else
        {
            i = second;
            j = first;
        }
    }

    int i;
    int j;
};

bool operator<(const CollisionPair& c1, const CollisionPair& c2)
{
    if (c1.i < c2.i) return true;
    if (c1.i == c2.i) return c1.j < c2.j;
    return false;
}

bool operator==(const CollisionPair& c1, const CollisionPair& c2)
{
    return (c1.i == c2.i && c1.j == c2.j) || (c1.i == c2.j) && (c1.j == c1.i);
}

class Food
{
public:
    Vector2f pos;
};

class Application : public KeyListener
{
public:
    Cell spawnCell(Vector2f pos, Vector2f vel, float radius)
    {
        Cell c = Cell{ pos, vel, radius };

        {
            b2BodyDef circleBody;
            circleBody.type = b2_dynamicBody;
            circleBody.position.Set(pos.x, pos.y);
            c.body = _world->CreateBody(&circleBody);

            b2CircleShape circle;
            circle.m_p.Set(0, 0);
            circle.m_radius = radius;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circle;
            fixtureDef.density = 1.0f;
            fixtureDef.friction = 0.3f;
            //fixtureDef.restitution = 1.0f;

            c.body->CreateFixture(&fixtureDef);
        }

        return c;
    }

    void spawnCells(std::vector<Cell>& cells, b2World* _world, int count)
    {
        for (int i = 0; i < count; i++)
        {
            Vector2f randomPos = Vector2f(distribution(generator) * 20 - 10, distribution(generator) * 20 - 10);
            cells.push_back(spawnCell(randomPos, Vector2f(0), 0.02f));
        }
    }

    void init()
    {
        b2Vec2 gravity(0.0f, 0.0f);
        _world = new b2World(gravity);

        window.create("EvoTest", 1024, 1024);
        window.addKeyListener(this);
        window.enableVSync(true);

        spawnCells(cells, _world, 32000);
        food.push_back(spawnCell(Vector2f(-10.5f, 0), Vector2f(0), 0.6f));
        //food.push_back(spawnCell(Vector2f(-6.5f, 0), Vector2f(0), 0.6f));
        food[0].body->ApplyLinearImpulse(b2Vec2(10, 0), food[0].body->GetPosition(), true);
    }
    int fileIndex = 0;

    void update()
    {
        renderer.init(window.getWidth(), window.getHeight());

        Camera& camera = renderer.getCamera();
        camera.zNear = 0.1f;
        camera.zFar = 60.0f;
        camera.fovy = 60;
        camera.aspect = 1;
        camera.position = Vector3f(0, 0, 16);

        while (!window.shouldClose())
        {
            if (stop)
            {
                window.update();
                continue;
            }
            renderer.clear();

            float timeStep = 1.0f / 60.0f;

            int32 velocityIterations = 6;
            int32 positionIterations = 2;

            //if (food.size() < 1)
            //    food.push_back(spawnCell(Vector2f(-10.5f, 0), Vector2f(0), 0.6f));

            for (int i = 0; i < cells.size(); i++)
            {
                Cell& cell = cells[i];
                
                Vector2f force = randomDirection() * 0.0001f;

                //for (int j = 0; j < cells.size(); j++)
                //{
                //    if (i == j) continue;

                //    Cell& other = cells[j];
                //    if ((cell.pos - other.pos).length() < 0.1f)
                //    {
                //        force += (other.pos - cell.pos) * 0.01f;
                //    }
                //}

                //if ((forcePos - cell.pos).length() < 0.3f)
                //{
                //    force += (cell.pos - forcePos) * 0.05f;
                //}
                //force += (forcePos - cell.pos) * 0.007f;

                for (const Cell& f : food)
                {
                    if ((cell.pos - f.pos).length() < 0.1f)
                    {
                        force += (f.pos - cell.pos) * 0.001f;
                    }
                }
                //cell.body->ApplyForce(b2Vec2(force.x, force.y), b2Vec2(cell.pos.x, cell.pos.y), true);
                //cell.pos += cell.vel;
                //cell.pos += (Vector2f(0) - cell.pos) * 0.01f;
            }
            //for (int i = 0; i < food.size(); i++)
            //{
                //Cell& cell = food[0];
                //if (cell.pos.x < -8.5)
                //    cell.body->ApplyForce(b2Vec2(0.80, 0), b2Vec2(cell.pos.x, cell.pos.y), true);
            //}

            _world->Step(timeStep, velocityIterations, positionIterations);

            for (Cell& cell : cells)
            {
                cell.pos = Vector2f(cell.body->GetPosition().x, cell.body->GetPosition().y);
            }

            for (Cell& cell : food)
            {
                cell.pos = Vector2f(cell.body->GetPosition().x, cell.body->GetPosition().y);
            }

            for (const Cell& cell : cells)
            {
                renderer.drawCircle(cell.pos, cell.radius, Vector3f(1, 1, 1));
            }

            for (const Cell& f : food)
            {
                renderer.drawCircle(f.pos, f.radius, Vector3f(1, 1, 0));
            }

            renderer.update();

            std::stringstream ss;
            ss << std::string("Output/output_") << std::setfill('0') << std::setw(4) << std::to_string(fileIndex) << std::string(".png");
            std::string filePath = ss.str();
            //recorder.record(renderer, filePath);
            fileIndex++;

            window.update();
        }
    }

    virtual void onKeyPressed(int key, int mods) override
    {
        stop = !stop;
    }

    virtual void onKeyReleased(int key, int mods) override
    {

    }

private:
    Window window;
    Renderer renderer;
    Recorder recorder;

    bool stop = true;

    std::vector<Cell> cells;

    std::vector<Cell> food;
    
    b2World* _world;
};

int main()
{
    Application application;
    application.init();
    application.update();
}
