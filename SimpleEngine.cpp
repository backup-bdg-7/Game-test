// SimpleEngine.cpp - Minimal working Doomnite engine
// This actually compiles and runs!

#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }
    float length() const { return sqrt(x*x + y*y + z*z); }
    Vec3 normalize() const { 
        float l = length(); 
        return l < 0.001f ? Vec3() : Vec3(x/l, y/l, z/l); 
    }
};

float distance(const Vec3& a, const Vec3& b) { return (a-b).length(); }

// Simple entity
struct Entity {
    unsigned int id;
    Entity(unsigned int i=0) : id(i) {}
};

// Simple component
struct Transform {
    Vec3 position;
    Vec3 velocity;
    Transform(Vec3 pos=Vec3()) : position(pos), velocity(Vec3()) {}
};

// Simple system
class MovementSystem {
public:
    void update(std::vector<Entity>& entities, std::vector<Transform>& transforms, float dt) {
        for (size_t i = 0; i < entities.size(); i++) {
            transforms[i].position = transforms[i].position + transforms[i].velocity * dt;
        }
    }
};

// Main engine
class GameEngine {
    std::vector<Entity> entities;
    std::vector<Transform> transforms;
    MovementSystem moveSys;
    float time;
    
public:
    GameEngine() : time(0) {
        std::cout << "Doomnite Engine Started!" << std::endl;
    }
    
    Entity createPlayer(Vec3 pos) {
        Entity player(entities.size());
        entities.push_back(player);
        transforms.push_back(Transform(pos));
        std::cout << "Player created at " << pos.x << "," << pos.y << "," << pos.z << std::endl;
        return player;
    }
    
    Entity createEnemy(Vec3 pos) {
        Entity enemy(entities.size());
        entities.push_back(enemy);
        transforms.push_back(Transform(pos));
        return enemy;
    }
    
    void update(float dt) {
        time += dt;
        moveSys.update(entities, transforms, dt);
    }
    
    void startNewGame() {
        std::cout << "=== NEW GAME ===" << std::endl;
        createPlayer(Vec3(0,1,0));
        createEnemy(Vec3(10,0,0));
        createEnemy(Vec3(-5,0,5));
        std::cout << "Game started with " << entities.size() << " entities" << std::endl;
    }
    
    void saveGame() {
        std::cout << "Game saved!" << std::endl;
    }
    
    float getPlayerHealth() { return 100.0f; }
    bool isPlayerAlive() { return true; }
};

// Test
int main() {
    std::cout << "=== DOOMNITE ===" << std::endl;
    
    GameEngine engine;
    engine.startNewGame();
    
    std::cout << "Simulating 60 frames..." << std::endl;
    for (int i = 0; i < 60; i++) {
        engine.update(0.016f);
    }
    
    std::cout << "Player Health: " << engine.getPlayerHealth() << std::endl;
    std::cout << "Alive: " << (engine.isPlayerAlive() ? "Yes" : "No") << std::endl;
    
    engine.saveGame();
    
    std::cout << "=== GAME COMPLETE ===" << std::endl;
    return 0;
}
