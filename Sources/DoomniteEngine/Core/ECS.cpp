// ECS.cpp - C++ Entity Component System Implementation

#include "ECS.hpp"

namespace Doomnite {

// MARK: - Entity Manager Implementation

uint32_t EntityManager::nextId = 0;
std::vector<uint32_t> EntityManager::freeIds;
std::vector<Entity> EntityManager::entities;

// ComponentStorage helper methods

ComponentStorage::ComponentContainer* ComponentStorage::findContainer(std::type_index type) {
    for (auto& container : containers) {
        if (container.type == type) {
            return &container;
        }
    }
    return nullptr;
}

// MovementSystem implementation

void MovementSystem::update(EntityManager& entities, float deltaTime) {
    auto allTransforms = entities.componentStorage.getAllComponents<TransformComponent>();
    
    for (auto& pair : allTransforms) {
        Entity entity = pair.first;
        auto transform = pair.second;
        auto velocity = entities.componentStorage.getComponent<VelocityComponent>(entity);
        
        if (!velocity) continue;
        
        transform->position += velocity->velocity * deltaTime;
        transform->rotation += velocity->angularVelocity * deltaTime;
        
        // Update back in storage
        entities.componentStorage.addComponent<TransformComponent>(entity, transform);
    }
}

} // namespace Doomnite
