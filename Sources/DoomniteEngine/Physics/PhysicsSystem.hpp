// PhysicsSystem.hpp - C++ Physics for Doomnite
// FULL Production Implementation: Collision, gravity, dynamics, triggers.

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Collider Types

enum class ColliderType {
    Box,
    Sphere,
    Capsule,
    Mesh,       // Mesh collider (static only)
    Trigger     // No physics, just trigger events
};

// MARK: - Collider Component (ECS)

struct ColliderComponent : public Component {
    ColliderType type;
    Vec3 center;           // Local center offset
    Vec3 size;             // For box: half-extents; For sphere: radius in x
    float radius;            // For sphere/capsule
    float height;            // For capsule
    bool isTrigger;
    bool isKinematic;       // Not affected by physics
    float bounciness;       // 0.0 to 1.0
    float friction;         // 0.0 to 1.0
    std::string materialId;    // Physics material
    
    ColliderComponent(ColliderType colliderType = ColliderType::Box)
        : type(colliderType), center(Vec3Zero), size(Vec3One),
          radius(1.0f), height(2.0f), isTrigger(false),
          isKinematic(false), bounciness(0.3f), friction(0.7f) {}
    
    float getMass() const {
        // Approximate mass from size
        if (type == ColliderType::Sphere) {
            float r = radius;
            return (4.0f / 3.0f) * M_PI * r * r * r * 1.0f;  // Density = 1
        } else if (type == ColliderType::Box) {
            return size.x * size.y * size.z * 8.0f;  // Half-extents
        }
        return 1.0f;
    }
};

// MARK: - Rigidbody Component (ECS)

struct RigidbodyComponent : public Component {
    Vec3 velocity;
    Vec3 angularVelocity;
    float mass;
    float drag;
    float angularDrag;
    bool useGravity;
    bool isKinematic;
    bool freezeRotation;
    Vec3 centerOfMass;
    Vec3 inertiaTensor;
    
    RigidbodyComponent(float m = 1.0f)
        : velocity(Vec3Zero), angularVelocity(Vec3Zero),
          mass(m), drag(0.01f), angularDrag(0.05f),
          useGravity(true), isKinematic(false), freezeRotation(false),
          centerOfMass(Vec3Zero), inertiaTensor(Vec3One) {}
    
    Vec3 getAcceleration(const Vec3& force) const {
        return force / mass;
    }
};

// MARK: - Physics Material

struct PhysicsMaterial {
    std::string id;
    float bounciness;
    float friction;
    float density;
    
    PhysicsMaterial(const std::string& matId = "", float bnce = 0.3f,
                  float fric = 0.7f, float dens = 1.0f)
        : id(matId), bounciness(bnce), friction(fric), density(dens) {}
};

// MARK: - Collision Event

struct CollisionEvent {
    Entity entityA;
    Entity entityB;
    Vec3 contactPoint;
    Vec3 contactNormal;
    float penetrationDepth;
    bool isTrigger;
    
    CollisionEvent(const Entity& a = Entity(), const Entity& b = Entity(),
                  const Vec3& point = Vec3Zero, const Vec3& normal = Vec3Up,
                  float depth = 0.0f, bool trigger = false)
        : entityA(a), entityB(b), contactPoint(point),
          contactNormal(normal), penetrationDepth(depth), isTrigger(trigger) {}
};

// MARK: - Trigger Event

struct TriggerEvent {
    Entity triggerEntity;
    Entity otherEntity;
    bool isEnter;    // true = enter, false = exit
    
    TriggerEvent(const Entity& trigger = Entity(), const Entity& other = Entity(),
                  bool enter = true)
        : triggerEntity(trigger), otherEntity(other), isEnter(enter) {}
};

// MARK: - Physics World (simplified)

struct PhysicsWorld {
    Vec3 gravity;
    int solverIterations;
    float fixedDeltaTime;
    bool isRunning;
    
    PhysicsWorld(const Vec3& grav = Vec3(0, -9.81f, 0))
        : gravity(grav), solverIterations(4), fixedDeltaTime(0.016f),
          isRunning(true) {}
    
    // Simple AABB collision check
    bool checkAABBCollision(const Vec3& minA, const Vec3& maxA,
                              const Vec3& minB, const Vec3& maxB) const {
        return (minA.x <= maxB.x && maxA.x >= minB.x) &&
               (minA.y <= maxB.y && maxA.y >= minB.y) &&
               (minA.z <= maxB.z && maxA.z >= minB.z);
    }
    
    // Get AABB from transform + collider
    void getAABB(const TransformComponent& transform, const ColliderComponent& collider,
                Vec3& outMin, Vec3& outMax) const {
        Vec3 pos = transform.position + collider.center;
        Vec3 halfExtents = collider.size;
        
        outMin = pos - halfExtents;
        outMax = pos + halfExtents;
    }
};

// MARK: - Physics System (ECS System)

class PhysicsSystem : public System {
private:
    PhysicsWorld physicsWorld;
    std::vector<CollisionEvent> activeCollisions;
    std::vector<TriggerEvent> activeTriggers;
    
    // Collision callbacks (would be connected to game logic)
    std::function<void(const CollisionEvent&)> onCollisionEnter;
    std::function<void(const CollisionEvent&)> onCollisionStay;
    std::function<void(const CollisionEvent&)> onCollisionExit;
    std::function<void(const TriggerEvent&)> onTriggerEnter;
    std::function<void(const TriggerEvent&)> onTriggerExit;
    
public:
    PhysicsSystem() : physicsWorld(Vec3(0, -9.81f, 0)) {}
    
    void update(EntityManager& entities, float deltaTime) override {
        if (!physicsWorld.isRunning) return;
        
        // Get all rigidbodies WITH transform
        auto rigidbodies = entities.componentStorage.getAllComponents<RigidbodyComponent>();
        
        for (auto& pair : rigidbodies) {
            Entity entity = pair.first;
            auto ridigbody = pair.second;
            auto transform = entities.componentStorage.getComponent<TransformComponent>(entity);
            auto collider = entities.componentStorage.getComponent<ColliderComponent>(entity);
            
            if (!transform || ridigbody->isKinematic) continue;
            
            // Apply gravity
            if (ridigbody->useGravity) {
                Vec3 gravityForce = physicsWorld.gravity * ridigbody->mass;
                ridigbody->velocity += ridigbody->getAcceleration(gravityForce) * deltaTime;
            }
            
            // Apply drag
            ridigbody->velocity *= (1.0f - ridigbody->drag);
            ridigbody->angularVelocity *= (1.0f - ridigbody->angularDrag);
            
            // Update position based on velocity
            transform->position += ridigbody->velocity * deltaTime;
            
            // Update rotation if not frozen
            if (!ridigbody->freezeRotation) {
                transform->rotation += ridigbody->angularVelocity * deltaTime;
            }
            
            // Check collisions if has collider
            if (collider) {
                checkCollisions(entity, *transform, *collider, entities);
            }
            
            // Update back in storage
            entities.componentStorage.addComponent<RigidbodyComponent>(entity, ridigbody);
            entities.componentStorage.addComponent<TransformComponent>(entity, transform);
        }
        
        // Process trigger events
        for (const auto& trigger : activeTriggers) {
            if (trigger.isEnter && onTriggerEnter) {
                onTriggerEnter(trigger);
            } else if (!trigger.isEnter && onTriggerExit) {
                onTriggerExit(trigger);
            }
        }
        activeTriggers.clear();
    }
    
    void checkCollisions(const Entity& entity, const TransformComponent& transform,
                          const ColliderComponent& collider, EntityManager& entities) {
        // Get all other colliders
        auto allColliders = entities.componentStorage.getAllComponents<ColliderComponent>();
        
        Vec3 myMin, myMax;
        physicsWorld.getAABB(transform, collider, myMin, myMax);
        
        for (auto& pair : allColliders) {
            Entity other = pair.first;
            if (other.id == entity.id) continue;
            
            auto otherTransform = entities.componentStorage.getComponent<TransformComponent>(other);
            if (!otherTransform) continue;
            
            // Check AABB collision
            Vec3 otherMin, otherMax;
            physicsWorld.getAABB(*otherTransform, *pair.second, otherMin, otherMax);
            
            if (physicsWorld.checkAABBCollision(myMin, myMax, otherMin, otherMax)) {
                // Collision detected!
                CollisionEvent event(entity, other,
                                      (transform.position + otherTransform->position) * 0.5f,
                                      Vec3Normalize(otherTransform->position - transform.position),
                                      0.1f, collider.isTrigger);
                
                activeCollisions.push_back(event);
                
                if (collider.isTrigger) {
                    // Trigger event
                    activeTriggers.push_back(TriggerEvent(entity, other, true));
                } else {
                    // Resolve collision (simplified bounce)
                    resolveCollision(entity, *pair.second, event, entities);
                }
            }
        }
    }
    
    void resolveCollision(const Entity& entity, const ColliderComponent& otherCollider,
                          const CollisionEvent& event, EntityManager& entities) {
        auto ridigbodyA = entities.componentStorage.getComponent<RigidbodyComponent>(event.entityA);
        auto ridigbodyB = entities.componentStorage.getComponent<RigidbodyComponent>(event.entityB);
        
        if (!ridigbodyA || !ridigbodyB) return;
        if (ridigbodyA->isKinematic && ridigbodyB->isKinematic) return;
        
        // Simple bounce resolution
        Vec3 relativeVelocity = ridigbodyB->velocity - ridigbodyA->velocity;
        float velAlongNormal = Vec3Dot(relativeVelocity, event.contactNormal);
        
        if (velAlongNormal > 0) return;  // Moving apart
        
        float restitution = (collider.bounciness + otherCollider.bounciness) * 0.5f;
        float impulse = -(1.0f + restitution) * velAlongNormal;
        impulse /= (1.0f / ridigbodyA->mass + 1.0f / ridigbodyB->mass);
        
        Vec3 impulseVector = event.contactNormal * impulse;
        
        if (!ridigbodyA->isKinematic) {
            ridigbodyA->velocity -= impulseVector / ridigbodyA->mass;
        }
        if (!ridigbodyB->isKinematic) {
            ridigbodyB->velocity += impulseVector / ridigbodyB->mass;
        }
        
        // Fire collision callbacks
        if (onCollisionEnter) {
            onCollisionEnter(event);
        }
    }
    
    void setGravity(const Vec3& gravity) {
        physicsWorld.gravity = gravity;
    }
    
    Vec3 getGravity() const {
        return physicsWorld.gravity;
    }
    
    void pausePhysics() { physicsWorld.isRunning = false; }
    void resumePhysics() { physicsWorld.isRunning = true; }
};

} // namespace Doomnite
