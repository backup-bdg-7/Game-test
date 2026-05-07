// FixedECS.hpp - Working C++ ECS
#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>

namespace Doomnite {

struct Entity {
    uint32_t id;
    Entity(uint32_t i=0) : id(i) {}
    bool operator==(const Entity& o) const { return id == o.id; }
};

class Component {
public:
    virtual ~Component() = default;
};

template<typename T>
class ComponentStorage {
    std::unordered_map<uint32_t, std::shared_ptr<Component>> components;
public:
    void addComponent(const Entity& e, std::shared_ptr<T> c) {
        components[e.id] = c;
    }
    
    std::shared_ptr<T> getComponent(const Entity& e) {
        auto it = components.find(e.id);
        if (it == components.end()) return nullptr;
        return std::dynamic_pointer_cast<T>(it->second);
    }
    
    std::vector<std::pair<Entity, std::shared_ptr<T>>> getAll() {
        std::vector<std::pair<Entity, std::shared_ptr<T>>> result;
        for (auto& p : components) {
            result.push_back({Entity(p.first), std::dynamic_pointer_cast<T>(p.second)});
        }
        return result;
    }
};

class EntityManager {
    uint32_t nextId = 0;
    std::vector<uint32_t> freeIds;
public:
    Entity createEntity() {
        uint32_t id;
        if (!freeIds.empty()) {
            id = freeIds.back();
            freeIds.pop_back();
        } else {
            id = nextId++;
        }
        return Entity(id);
    }
    
    void destroyEntity(const Entity& e) {
        freeIds.push_back(e.id);
    }
};

class System {
public:
    virtual ~System() = default;
    virtual void update(EntityManager& em, float dt) = 0;
};

class GameEngine {
    EntityManager entityManager;
public:
    GameEngine() {
        std::cout << "GameEngine created!" << std::endl;
    }
    
    Entity createPlayer() {
        Entity player = entityManager.createEntity();
        std::cout << "Player created: " << player.id << std::endl;
        return player;
    }
    
    void update(float dt) {
        // Would update all systems
    }
    
    EntityManager& getEntityManager() { return entityManager; }
};

} // namespace Doomnite
