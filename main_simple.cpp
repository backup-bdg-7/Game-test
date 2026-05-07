// main_simple.cpp - Basic C++ Test
// Tests simple compilation

#include <iostream>
#include <memory>
#include <vector>

struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
};

class Entity {
public:
    unsigned int id;
    Entity(unsigned int i=0) : id(i) {}
};

class EntityManager {
    unsigned int nextId;
public:
    EntityManager() : nextId(0) {}
    
    Entity createEntity() {
        return Entity(nextId++);
    }
};

class GameEngine {
    EntityManager entityManager;
public:
    GameEngine() {
        std::cout << "GameEngine created!" << std::endl;
    }
    
    Entity createPlayer(const Vec3& position) {
        Entity player = entityManager.createEntity();
        std::cout << "Player created with ID: " << player.id << std::endl;
        return player;
    }
    
    void update(float deltaTime) {
        // Would update all systems
    }
    
    void startNewGame() {
        std::cout << "New game started!" << std::endl;
        Entity player = createPlayer(Vec3(0, 1, 0));
    }
};

int main() {
    std::cout << "=== Doomnite Engine Test ===" << std::endl;
    
    GameEngine engine;
    engine.startNewGame();
    
    std::cout << "Simulating 60 frames..." << std::endl;
    for (int i=0; i<60; i++) {
        engine.update(0.016f);
    }
    
    std::cout << "Test passed!" << std::endl;
    return 0;
}
