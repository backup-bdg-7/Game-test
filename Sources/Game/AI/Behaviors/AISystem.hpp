// AISystem.hpp - C++ AI System for Doomnite
// Behavior trees, navigation mesh, squad tactics, enemy spawning

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Behavior Tree Nodes

enum class NodeStatus {
    Running,
    Success,
    Failure
};

class BehaviorNode {
public:
    virtual ~BehaviorNode() = default;
    virtual NodeStatus tick(EntityManager& entities, const Entity& entity, float deltaTime) = 0;
};

class SequenceNode : public BehaviorNode {
    std::vector<std::shared_ptr<BehaviorNode>> children;
    
public:
    void addChild(std::shared_ptr<BehaviorNode> child) { children.push_back(child); }
    
    NodeStatus tick(EntityManager& entities, const Entity& entity, float deltaTime) override {
        for (auto& child : children) {
            NodeStatus status = child->tick(entities, entity, deltaTime);
            if (status != NodeStatus::Success) {
                return status;
            }
        }
        return NodeStatus::Success;
    }
};

class SelectorNode : public BehaviorNode {
    std::vector<std::shared_ptr<BehaviorNode>> children;
    
public:
    void addChild(std::shared_ptr<BehaviorNode> child) { children.push_back(child); }
    
    NodeStatus tick(EntityManager& entities, const Entity& entity, float deltaTime) override {
        for (auto& child : children) {
            NodeStatus status = child->tick(entities, entity, deltaTime);
            if (status != NodeStatus::Failure) {
                return status;
            }
        }
        return NodeStatus::Failure;
    }
};

class ConditionNode : public BehaviorNode {
    std::function<bool(EntityManager&, const Entity&)> condition;
    
public:
    ConditionNode(std::function<bool(EntityManager&, const Entity&)> cond) : condition(cond) {}
    
    NodeStatus tick(EntityManager& entities, const Entity& entity, float deltaTime) override {
        return condition(entities, entity) ? NodeStatus::Success : NodeStatus::Failure;
    }
};

class ActionNode : public BehaviorNode {
    std::function<NodeStatus(EntityManager&, const Entity&, float)> action;
    
public:
    ActionNode(std::function<NodeStatus(EntityManager&, const Entity&, float)> act) : action(act) {}
    
    NodeStatus tick(EntityManager& entities, const Entity& entity, float deltaTime) override {
        return action(entities, entity, deltaTime);
    }
};

// MARK: - Enemy AI Component (ECS)

struct EnemyAIComponent : public Component {
    std::shared_ptr<BehaviorNode> behaviorTree;
    EnemyState currentState;
    Entity targetEntity;
    float stateTimer;
    float detectionRadius;
    float attackRadius;
    float patrolSpeed;
    float chaseSpeed;
    Vec3 patrolPoint1;
    Vec3 patrolPoint2;
    bool patrollingToPoint1;
    
    EnemyAIComponent(float detectRadius = 20.0f, float atkRadius = 3.0f)
        : currentState(EnemyState::Idle), stateTimer(0.0f),
          detectionRadius(detectRadius), attackRadius(atkRadius),
          patrolSpeed(2.0f), chaseSpeed(5.0f),
          patrolPoint1(-10, 0, 0), patrolPoint2(10, 0, 0),
          patrollingToPoint1(true) {
        buildBehaviorTree();
    }
    
    void buildBehaviorTree() {
        auto root = std::make_shared<SelectorNode>();
        
        // Attack sequence
        auto attackSequence = std::make_shared<SequenceNode>();
        attackSequence->addChild(std::make_shared<ConditionNode>(
            [this](EntityManager& e, const Entity& entity) {
                return currentState == EnemyState::Chasing && targetEntity.id != 0;
            }));
        attackSequence->addChild(std::make_shared<ActionNode>(
            [this](EntityManager& e, const Entity& entity, float dt) {
                // Perform attack
                return NodeStatus::Success;
            }));
        
        // Chase sequence
        auto chaseSequence = std::make_shared<SequenceNode>();
        chaseSequence->addChild(std::make_shared<ConditionNode>(
            [this](EntityManager& e, const Entity& entity) {
                return currentState == EnemyState::Patrolling || currentState == EnemyState::Idle;
            }));
        chaseSequence->addChild(std::make_shared<ActionNode>(
            [this](EntityManager& e, const Entity& entity, float dt) {
                // Find and chase player
                return NodeStatus::Success;
            }));
        
        // Patrol action
        auto patrolAction = std::make_shared<ActionNode>(
            [this](EntityManager& e, const Entity& entity, float dt) {
                // Patrol between points
                return NodeStatus::Running;
            });
        
        root->addChild(attackSequence);
        root->addChild(chaseSequence);
        root->addChild(patrolAction);
        
        behaviorTree = root;
    }
};

// MARK: - Squad Component (for squad tactics)

struct SquadComponent : public Component {
    std::vector<Entity> members;
    Entity leader;
    Vec3 squadPosition;
    float formationRadius;
    bool isAggressive;
    
    SquadComponent(float formRadius = 5.0f, bool aggressive = true)
        : formationRadius(formRadius), isAggressive(aggressive) {}
    
    void addMember(const Entity& member) {
        if (members.empty()) {
            leader = member;
        }
        members.push_back(member);
    }
    
    void removeMember(const Entity& member) {
        members.erase(std::remove(members.begin(), members.end(), member), members.end());
        if (leader.id == member.id && !members.empty()) {
            leader = members[0];  // Assign new leader
        }
    }
    
    Vec3 getFormationPosition(int index) const {
        if (members.empty()) return squadPosition;
        
        float angle = (2.0f * M_PI * index) / members.size();
        return Vec3(
            squadPosition.x + formationRadius * cosf(angle),
            squadPosition.y,
            squadPosition.z + formationRadius * sinf(angle)
        );
    }
};

// MARK: - Navigation Component

struct NavigationComponent : public Component {
    std::vector<Vec3> path;
    int currentPathIndex;
    bool hasPath;
    float repathTimer;
    
    NavigationComponent() : currentPathIndex(0), hasPath(false), repathTimer(0.0f) {}
    
    void setPath(const std::vector<Vec3>& newPath) {
        path = newPath;
        currentPathIndex = 0;
        hasPath = !path.empty();
    }
    
    Vec3 getCurrentTarget() const {
        if (currentPathIndex < static_cast<int>(path.size())) {
            return path[currentPathIndex];
        }
        return Vec3Zero;
    }
    
    void advanceToNext() {
        currentPathIndex++;
        if (currentPathIndex >= static_cast<int>(path.size())) {
            hasPath = false;
        }
    }
};

// MARK: - AI System

class AISystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        updateEnemies(entities, deltaTime);
        updateSquads(entities, deltaTime);
        updateNavigation(entities, deltaTime);
    }
    
private:
    void updateEnemies(EntityManager& entities, float deltaTime) {
        auto enemies = entities.componentStorage.getAllComponents<EnemyAIComponent>();
        
        for (auto& pair : enemies) {
            Entity entity = pair.first;
            auto ai = pair.second;
            
            // Tick behavior tree
            if (ai->behaviorTree) {
                ai->behaviorTree->tick(entities, entity, deltaTime);
            }
            
            // Update state timer
            ai->stateTimer += deltaTime;
            
            // Simple state-based AI (fallback if no behavior tree)
            if (!ai->behaviorTree) {
                updateSimpleAI(entity, ai, entities, deltaTime);
            }
        }
    }
    
    void updateSimpleAI(const Entity& entity, std::shared_ptr<EnemyAIComponent> ai,
                       EntityManager& entities, float deltaTime) {
        auto transform = entities.componentStorage.getComponent<TransformComponent>(entity);
        if (!transform) return;
        
        switch (ai->currentState) {
            case EnemyState::Idle:
                // Check for player in detection radius
                if (detectPlayer(transform->position, ai->detectionRadius, entities)) {
                    ai->currentState = EnemyState::Chasing;
                }
                break;
                
            case EnemyState::Patrolling:
                updatePatrol(entity, transform, ai, deltaTime);
                if (detectPlayer(transform->position, ai->detectionRadius, entities)) {
                    ai->currentState = EnemyState::Chasing;
                }
                break;
                
            case EnemyState::Chasing:
                if (chasePlayer(entity, transform, ai, entities, deltaTime)) {
                    ai->currentState = EnemyState::Attacking;
                }
                if (!detectPlayer(transform->position, ai->detectionRadius * 1.5f, entities)) {
                    ai->currentState = EnemyState::Patrolling;
                }
                break;
                
            case EnemyState::Attacking:
                if (attackPlayer(entity, transform, ai, entities, deltaTime)) {
                    // Attack performed
                }
                if (!detectPlayer(transform->position, ai->attackRadius * 1.5f, entities)) {
                    ai->currentState = EnemyState::Chasing;
                }
                break;
                
            case EnemyState::Fleeing:
                // Flee from player
                break;
                
            case EnemyState::Dead:
                // Do nothing
                break;
        }
    }
    
    void updatePatrol(const Entity& entity, std::shared_ptr<TransformComponent> transform,
                     std::shared_ptr<EnemyAIComponent> ai, float deltaTime) {
        Vec3 target = ai->patrollingToPoint1 ? ai->patrolPoint1 : ai->patrolPoint2;
        
        Vec3 direction = target - transform->position;
        direction.y = 0;  // Keep on ground
        
        if (Vec3Length(direction) < 0.5f) {
            // Reached patrol point, switch to other
            ai->patrollingToPoint1 = !ai->patrollingToPoint1;
        } else {
            // Move towards target
            Vec3 normalizedDir = Vec3Normalize(direction);
            transform->position += normalizedDir * ai->patrolSpeed * deltaTime;
            
            // Face movement direction
            if (Vec3Length(direction) > 0.1f) {
                transform->rotation.y = atan2f(normalizedDir.x, normalizedDir.z);
            }
        }
        
        entities.componentStorage.addComponent<TransformComponent>(entity, transform);
    }
    
    bool chasePlayer(const Entity& entity, std::shared_ptr<TransformComponent> transform,
                    std::shared_ptr<EnemyAIComponent> ai, EntityManager& entities,
                    float deltaTime) {
        // Find player (simplified - would use spatial partition)
        auto allPlayers = entities.componentStorage.getAllComponents<PlayerDataComponent>();
        if (allPlayers.empty()) return false;
        
        Entity player = allPlayers[0].first;
        auto playerTransform = entities.componentStorage.getComponent<TransformComponent>(player);
        if (!playerTransform) return false;
        
        Vec3 direction = playerTransform->position - transform->position;
        direction.y = 0;
        
        float distance = Vec3Length(direction);
        
        // If within attack range, return true
        if (distance <= ai->attackRadius) {
            return true;
        }
        
        // Move towards player
        if (distance > 0.1f) {
            Vec3 normalizedDir = Vec3Normalize(direction);
            transform->position += normalizedDir * ai->chaseSpeed * deltaTime;
            
            // Face player
            transform->rotation.y = atan2f(normalizedDir.x, normalizedDir.z);
        }
        
        entities.componentStorage.addComponent<TransformComponent>(entity, transform);
        return false;
    }
    
    bool attackPlayer(const Entity& entity, std::shared_ptr<TransformComponent> transform,
                     std::shared_ptr<EnemyAIComponent> ai, EntityManager& entities,
                     float deltaTime) {
        // Perform attack (would damage player)
        // Reset attack cooldown, etc.
        return true;
    }
    
    bool detectPlayer(const Vec3& position, float radius, EntityManager& entities) {
        // Simplified - would use spatial partition
        auto allPlayers = entities.componentStorage.getAllComponents<PlayerDataComponent>();
        
        for (auto& pair : allPlayers) {
            Entity player = pair.first;
            auto playerTransform = entities.componentStorage.getComponent<TransformComponent>(player);
            if (!playerTransform) continue;
            
            if (Vec3Distance(position, playerTransform->position) <= radius) {
                return true;
            }
        }
        return false;
    }
    
    void updateSquads(EntityManager& entities, float deltaTime) {
        auto squads = entities.componentStorage.getAllComponents<SquadComponent>();
        
        for (auto& pair : squads) {
            Entity entity = pair.first;
            auto squad = pair.second;
            
            // Update squad position to average of members
            if (!squad->members.empty()) {
                Vec3 sum = Vec3Zero;
                for (const auto& member : squad->members) {
                    auto memberTransform = entities.componentStorage.getComponent<TransformComponent>(member);
                    if (memberTransform) {
                        sum += memberTransform->position;
                    }
                }
                squad->squadPosition = sum / static_cast<float>(squad->members.size());
            }
        }
    }
    
    void updateNavigation(EntityManager& entities, float deltaTime) {
        auto navigators = entities.componentStorage.getAllComponents<NavigationComponent>();
        
        for (auto& pair : navigators) {
            Entity entity = pair.first;
            auto nav = pair.second;
            
            if (!nav->hasPath) continue;
            
            auto transform = entities.componentStorage.getComponent<TransformComponent>(entity);
            if (!transform) continue;
            
            Vec3 target = nav->getCurrentTarget();
            Vec3 direction = target - transform->position;
            direction.y = 0;
            
            if (Vec3Length(direction) < 0.5f) {
                nav->advanceToNext();
            } else {
                // Move towards target
                Vec3 normalizedDir = Vec3Normalize(direction);
                transform->position += normalizedDir * 3.0f * deltaTime;  // Default speed
            }
            
            entities.componentStorage.addComponent<TransformComponent>(entity, transform);
        }
    }
};

} // namespace Doomnite
