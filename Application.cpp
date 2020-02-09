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

float randomAngle()
{
    return distribution(generator) * Math::TWO_PI;
}

Vector2f angleToVec(float angle)
{
    return Vector2f(cos(angle), sin(angle));
}

//class Cell
//{
//public:
//    Vector2f pos;
//    Vector2f vel;
//    float heading;
//    float radius;
//};

class Cell
{
public:
    Vector2f pos;
    Vector2f vel;
    float heading;
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

//class Application : public KeyListener
//{
//public:
//    void init()
//    {
//        window.create("EvoTest", 1024, 1024);
//        window.addKeyListener(this);
//        window.enableVSync(true);
//        
//        for (int i = 0; i < 1000; i++)
//        {
//            Vector2f randomPos = Vector2f(distribution(generator) * 2 - 1, distribution(generator) * 2 - 1);
//            cells.push_back(Cell{ randomPos * 2.5f, Vector2f(0), randomAngle(), 0.02f });
//        }
//    }
//
//    //void intersection(std::vector<CollisionPair>& nums1, std::vector<CollisionPair>& nums2, std::vector<CollisionPair> res) {
//    //    std::unordered_set<CollisionPair> m(nums1.begin(), nums1.end());
//    //    
//    //    for (auto a : nums2)
//    //    {
//    //        if (m.count(a)) {
//    //            res.push_back(a);
//    //            m.erase(a);
//    //        }
//    //    }
//    //}
//
//    bool findCollisions(std::vector<CollisionPair>& collisionPairs)
//    {
//        auto start = std::chrono::high_resolution_clock::now();
//        std::vector<CollisionPair> xCollisions;
//        std::vector<CollisionPair> yCollisions;
//
//        std::vector<int> indices(cells.size());
//        std::iota(indices.begin(), indices.end(), 0);
//        std::sort(indices.begin(), indices.end(), [this](unsigned int i1, unsigned int i2) { return cells[i1].pos.x < cells[i2].pos.x; });
//
//        for (int i = 0; i < indices.size() - 1; i++)
//        {
//            Cell& iCell = cells[indices[i]];
//            float maxExtent = iCell.pos.x + iCell.radius;
//
//            for (int j = i + 1; j < indices.size(); j++)
//            {
//                Cell& jCell = cells[indices[j]];
//
//                if (jCell.pos.x - jCell.radius > maxExtent) break;
//
//                CollisionPair c(indices[i], indices[j]);
//                xCollisions.push_back(c);
//                //xCollisions.insert();
//            }
//        }
//        //std::cout << veccol.size() << std::endl;
//        
//        //veccol.erase(std::unique(veccol.begin(), veccol.end()), veccol.end());
//
//        std::iota(indices.begin(), indices.end(), 0);
//        std::sort(indices.begin(), indices.end(), [this](unsigned int i1, unsigned int i2) { return cells[i1].pos.y < cells[i2].pos.y; });
//
//        for (int i = 0; i < indices.size() - 1; i++)
//        {
//            Cell& iCell = cells[indices[i]];
//            float maxExtent = iCell.pos.y + iCell.radius;
//
//            for (int j = i + 1; j < indices.size(); j++)
//            {
//                Cell& jCell = cells[indices[j]];
//
//                if (jCell.pos.y - jCell.radius > maxExtent) break;
//
//                yCollisions.push_back(CollisionPair(indices[i], indices[j]));
//            }
//        }
//
//        //std::cout << xCollisions.size() << std::endl;
//        //std::cout << yCollisions.size() << std::endl;
//
//        std::sort(xCollisions.begin(), xCollisions.end());
//        std::sort(yCollisions.begin(), yCollisions.end());
//
//        collisionPairs.resize(xCollisions.size());
//        auto ls = std::set_intersection(xCollisions.begin(), xCollisions.end(), yCollisions.begin(), yCollisions.end(), collisionPairs.begin());
//
//        ////std::cout << (ls - collisionPairs.begin()) << std::endl;
//
//        //intersection(xCollisions, yCollisions, collisionPairs);
//
//        for (auto it = collisionPairs.begin(); it != ls; ++it)
//        //for (auto it = collisionPairs.begin(); it != collisionPairs.end(); ++it)
//        {
//            const auto pair = *it;
//            Cell& aCell = cells[pair.i];
//            Cell& bCell = cells[pair.j];
//            renderer.drawCircle(aCell.pos, 0.01f, Vector3f(1, 0, 0));
//            renderer.drawCircle(bCell.pos, 0.01f, Vector3f(1, 0, 0));
//        }
//        auto finish = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> elapsed = finish - start;
//        std::cout << "Sweep Elapsed time: " << elapsed.count() * 1000 << " ms\n";
//
//        return true;
//    }
//
//    bool colliding(Cell& a, Cell& b)
//    {
//        float dist = (b.pos - a.pos).length();
//        if (dist < a.radius + b.radius)
//            return true;
//        return false;
//    }
//
//    bool findCollisionsNaive(std::vector<CollisionPair>& collisionPairs)
//    {
//        auto start = std::chrono::high_resolution_clock::now();
//        for (int i = 0; i < cells.size()-1; i++)
//        {
//            Cell& aCell = cells[i];
//
//            for (int j = i + 1; j < cells.size(); j++)
//            {
//                Cell& bCell = cells[j];
//
//                //if (fabs(bCell.pos.x - aCell.pos.x) < aCell.radius + bCell.radius &&
//                //    fabs(bCell.pos.y - aCell.pos.y) < aCell.radius + bCell.radius)
//                if (colliding(aCell, bCell))
//                    collisionPairs.emplace_back(CollisionPair(i, j));
//            }
//        }
//
//        for (auto it = collisionPairs.begin(); it != collisionPairs.end(); ++it)
//        {
//            const auto pair = *it;
//            Cell& aCell = cells[pair.i];
//            Cell& bCell = cells[pair.j];
//            renderer.drawCircle(aCell.pos, 0.01f, Vector3f(1, 0, 0));
//            renderer.drawCircle(bCell.pos, 0.01f, Vector3f(1, 0, 0));
//        }
//        auto finish = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> elapsed = finish - start;
//        std::cout << "Naive Elapsed time: " << elapsed.count() * 1000 << " ms\n";
//        return true;
//    }
//
//    void resolveCollisions(std::vector<CollisionPair>& collisionPairs)
//    {
//        for (const CollisionPair& pair : collisionPairs)
//        {
//            Cell& iCell = cells[pair.i];
//            Cell& jCell = cells[pair.j];
//
//            float overlap = iCell.radius + jCell.radius - (iCell.pos - jCell.pos).length() + 0.01f;
//            iCell.pos = iCell.pos + normalize(iCell.pos - jCell.pos) * (overlap / 2);
//            jCell.pos = jCell.pos + normalize(jCell.pos - iCell.pos) * (overlap / 2);
//        }
//    }
//
//    void update()
//    {
//        renderer.init();
//
//        Camera& camera = renderer.getCamera();
//        camera.zNear = 0.1f;
//        camera.zFar = 30.0f;
//        camera.fovy = 60;
//        camera.aspect = 1;
//        camera.position = Vector3f(0, 0, 5);
//
//        while (!window.shouldClose())
//        {
//            glClearColor(0, 0, 0, 1);
//            glClear(GL_COLOR_BUFFER_BIT);
//            if (stop)
//            {
//                window.update();
//                continue;
//            }
//            
//            for (Cell& cell : cells)
//            {
//                cell.heading += 0.01f;
//                cell.vel += randomDirection() * 0.0001f;//angleToVec(cell.heading) * 0.001f;
//                cell.pos += cell.vel;
//                cell.pos += (Vector2f(0) - cell.pos) * 0.01f;
//            }
//            std::vector<CollisionPair> cind;
//            //findCollisions(cind);
//            do
//            {
//                cind.clear();
//                findCollisionsNaive(cind);
//                //findCollisions(cind);
//
//                resolveCollisions(cind);
//                //std::cout << cind.size() << std::endl;
//            } while (!cind.empty());
//
//
//            //for (int i = 0; i < cells.size(); i++)
//            //{
//            //    Cell& iCell = cells[i];
//
//            //    iCell.vel += randomDirection() * 0.0001f;//angleToVec(cell.heading) * 0.001f;
//            //    Vector2f newPos = iCell.pos + iCell.vel;
//
//            //    for (int j = 0; j < cells.size(); j++)
//            //    {
//            //        if (i == j) continue;
//            //        Cell& jCell = cells[j];
//
//            //        if ((jCell.pos - newPos).sqrMagnitude() <= pow(0.04f, 2))
//            //        {
//            //            newPos = jCell.pos + normalize(newPos - jCell.pos) * (0.04f);
//            //        }
//            //    }
//
//            //    iCell.pos = newPos;
//            //}
//            //Sleep(100);
//
//            for (const Cell& cell : cells)
//            {
//                renderer.drawCircle(cell.pos, 0.02f, Vector3f(1, 1, 1));
//            }
//
//            for (const Food& food : foods)
//            {
//                renderer.drawCircle(food.pos, 0.02f, Vector3f(1, 1, 1));
//            }
//
//            renderer.update();
//
//            window.update();
//        }
//    }
//
//    virtual void onKeyPressed(int key, int mods) override
//    {
//        stop = !stop;
//    }
//
//    virtual void onKeyReleased(int key, int mods) override
//    {
//
//    }
//
//private:
//    Window window;
//    Renderer renderer;
//
//    bool stop = true;
//
//    std::vector<Cell> cells;
//    std::vector<Food> foods;
//};

void spawnCells(std::vector<Cell>& cells, b2World* _world, int count)
{
    for (int i = 0; i < count; i++)
    {
        Vector2f randomPos = Vector2f(distribution(generator) * 4 - 2, distribution(generator) * 4 - 2);
        cells.push_back(Cell{ randomPos * 2.5f, Vector2f(0), randomAngle(), 0.02f });

        {
            b2BodyDef circleBody;
            circleBody.type = b2_dynamicBody;
            circleBody.position.Set(randomPos.x, randomPos.y);
            cells[i].body = _world->CreateBody(&circleBody);

            b2CircleShape circle;
            circle.m_p.Set(0, 0);
            circle.m_radius = 0.02f;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circle;
            fixtureDef.density = 1.0f;
            fixtureDef.friction = 0.3f;

            cells[i].body->CreateFixture(&fixtureDef);
        }
    }
}

class Application : public KeyListener
{
public:
    void init()
    {
        b2Vec2 gravity(0.0f, 0.0f);
        _world = new b2World(gravity);

        window.create("EvoTest", 1024, 1024);
        window.addKeyListener(this);
        window.enableVSync(true);

        spawnCells(cells, _world, 1000);
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
        camera.position = Vector3f(0, 0, 5);

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

            Vector2f forcePos = Vector2f(distribution(generator) * 4 - 2, distribution(generator) * 4 - 2);

            //food.push_back(Food {});

            for (int i = 0; i < cells.size(); i++)
            {
                Cell& cell = cells[i];
                cell.heading += 0.01f;
                Vector2f force = randomDirection() * 0.0001f;

                for (int j = 0; j < cells.size(); j++)
                {
                    if (i == j) continue;

                    Cell& other = cells[j];
                    if ((cell.pos - other.pos).length() < 0.1f)
                    {
                        force += (other.pos - cell.pos) * 0.01f;
                    }
                }

                //if ((forcePos - cell.pos).length() < 0.3f)
                //{
                //    force += (cell.pos - forcePos) * 0.05f;
                //}
                //force += (forcePos - cell.pos) * 0.007f;

                cell.body->ApplyForce(b2Vec2(force.x, force.y), b2Vec2(cell.pos.x, cell.pos.y), true);
                //cell.pos += cell.vel;
                //cell.pos += (Vector2f(0) - cell.pos) * 0.01f;
            }

            _world->Step(timeStep, velocityIterations, positionIterations);

            for (Cell& cell : cells)
            {
                cell.pos = Vector2f(cell.body->GetPosition().x, cell.body->GetPosition().y);
            }

            for (const Cell& cell : cells)
            {
                renderer.drawCircle(cell.pos, 0.02f, Vector3f(1, 1, 1));
            }

            renderer.update();

            std::stringstream ss;
            ss << std::string("Output/output_") << std::setfill('0') << std::setw(3) << std::to_string(fileIndex) << std::string(".png");
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

    std::vector<Food> food;
    
    b2World* _world;
};

int main()
{
    Application application;
    application.init();
    application.update();
}
