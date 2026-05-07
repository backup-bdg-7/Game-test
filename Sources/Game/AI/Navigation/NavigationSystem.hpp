// NavigationSystem.hpp - C++ Navigation Mesh for Doomnite
// Handles pathfinding, waypoints, and navigation queries.

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Waypoint (Navigation point)

struct Waypoint {
    std::string id;
    Vec3 position;
    std::vector<std::string> connections;  // Connected waypoint IDs
    bool isAccessible;   // Can pathfind through here?
    std::string zoneId;       // Which zone this belongs to
    
    Waypoint(const std::string& waypointId = ", const Vec3& pos = Vec3Zero)
        : id(waypointId), position(pos), isAccessible(true) {}
    
    float distanceTo(const Vec3& point) const {
        return Vec3Distance(position, point);
    }
};

// MARK: - Navigation Mesh (Simplified)

struct NavMesh {
    std::string id;
    std::vector<Waypoint> waypoints;
    Vec3 boundsMin;
    Vec3 boundsMax;
    
    NavMesh(const std::string& meshId = ") : id(meshId) {
        boundsMin = Vec3Make(-1000, -100, -1000);
        boundsMax = Vec3Make(1000, 100, 1000);
    }
    
    Waypoint* getWaypoint(const std::string& waypointId) {
        for (auto& wp : waypoints) {
            if (wp.id == waypointId) return &wp;
        }
        return nullptr;
    }
    
    Waypoint* getNearestWaypoint(const Vec3& position) {
        Waypoint* nearest = nullptr;
        float nearestDist = INFINITY;
        
        for (auto& wp : waypoints) {
            float dist = wp.distanceTo(position);
            if (dist < nearestDist) {
                nearestDist = dist;
                nearest = &wp;
            }
        }
        return nearest;
    }
    
    std::vector<Vec3> findPath(const Vec3& start, const Vec3& end) {
        std::vector<Vec3> path;
        
        // Simplified: just go straight (would use A* on waypoints)
        path.push_back(start);
        
        // Add waypoint if available
        Waypoint* startWp = getNearestWaypoint(start);
        Waypoint* endWp = getNearestWaypoint(end);
        
        if (startWp && endWp && startWp->id != endWp->id) {
            // Would do A* pathfinding here
            path.push_back(startWp->position);
            path.push_back(endWp->position);
        }
        
        path.push_back(end);
        return path;
    }
    
    void addWaypoint(const Waypoint& waypoint) {
        waypoints.push_back(waypoint);
    }
    
    bool connectWaypoints(const std::string& id1, const std::string& id2) {
        Waypoint* wp1 = getWaypoint(id1);
        Waypoint* wp2 = getWaypoint(id2);
        
        if (!wp1 || !wp2) return false;
        
        wp1->connections.push_back(id2);
        wp2->connections.push_back(id1);
        return true;
    }
};

// MARK: - Navigation Component (ECS)

struct NavigationComponent : public Component {
    std::string currentWaypointId;
    std::vector<Vec3> currentPath;
    int currentPathIndex;
    bool hasPath;
    float repathTimer;
    float speed;
    
    NavigationComponent(float moveSpeed = 3.0f)
        : currentPathIndex(0), hasPath(false), repathTimer(0.0f),
          speed(moveSpeed) {}
    
    void setPath(const std::vector<Vec3>& newPath) {
        currentPath = newPath;
        currentPathIndex = 0;
        hasPath = !currentPath.empty();
    }
    
    Vec3 getCurrentTarget() const {
        if (currentPathIndex < static_cast<int>(currentPath.size())) {
            return currentPath[currentPathIndex];
        }
        return Vec3Zero;
    }
    
    void advanceToNext() {
        currentPathIndex++;
        if (currentPathIndex >= static_cast<int>(currentPath.size())) {
            hasPath = false;
        }
    }
};

// MARK: - Predefined Navigation Meshes

namespace NavigationMeshes {
    
    static NavMesh MainWorld("nav_main_world");
    static NavMesh Dungeon1("nav_dungeon_1");
    static NavMesh CityHub("nav_city_hub");
    
    static std::vector<NavMesh*> AllMeshes = {
        &MainWorld, &Dungeon1, &CityHub
    };
    
    static NavMesh* getMeshById(const std::string& id) {
        for (auto* mesh : AllMeshes) {
            if (mesh->id == id) return mesh;
        }
        return nullptr;
    }
}

// MARK: - Navigation System (ECS System)

class NavigationSystem : public System {
private:
    std::unordered_map<std::string, NavMesh> navMeshes;
    
public:
    NavigationSystem() {
        // Load predefined meshes
        for (auto* mesh : NavigationMeshes::AllMeshes) {
            navMeshes[mesh->id] = *mesh;
        }
    }
    
    void update(EntityManager& entities, float deltaTime) override {
        auto navigators = entities.componentStorage.getAllComponents<NavigationComponent>();
        
        for (auto& pair : navigators) {
            Entity entity = pair.first;
            auto nav = pair.second;
            
            if (!nav->hasPath) {
                // Find new path if needed
                // Would recalculate based on target
                continue;
            }
            
            auto transform = entities.componentStorage.getComponent<TransformComponent>(entity);
            if (!transform) continue;
            
            Vec3 target = nav->getCurrentTarget();
            Vec3 direction = target - transform->position;
            
            float distance = Vec3Length(direction);
            if (distance < 0.5f) {
                nav->advanceToNext();
                continue;
            }
            
            // Move towards target
            if (distance > 0.1f) {
                Vec3 normalizedDir = Vec3Normalize(direction);
                transform->position += normalizedDir * nav->speed * deltaTime;
                
                // Face movement direction
                if (normalizedDir.x != 0 || normalizedDir.z != 0) {
                    transform->rotation.y = atan2f(normalizedDir.x, normalizedDir.z);
                }
            }
            
            // Update back in storage
            entities.componentStorage.addComponent<TransformComponent>(entity, transform);
            entities.componentStorage.addComponent<NavigationComponent>(entity, nav);
        }
    }
    
    bool calculatePath(const std::string& meshId, const Vec3& start, const Vec3& end,
                       EntityManager& entities, const Entity& entity) {
        auto it = navMeshes.find(meshId);
        if (it == navMeshes.end()) return false;
        
        auto* navComp = entities.componentStorage.getComponent<NavigationComponent>(entity);
        if (!navComp) {
            navComp = std::make_shared<NavigationComponent>();
            entities.componentStorage.addComponent<NavigationComponent>(entity, navComp);
        }
        
        std::vector<Vec3> path = it->second.findPath(start, end);
        navComp->setPath(path);
        
        return !path.empty();
    }
    
    void addMesh(const NavMesh& mesh) {
        navMeshes[mesh.id] = mesh;
    }
    
    void clearAll() {
        navMeshes.clear();
    }
};

} // namespace Doomnite
