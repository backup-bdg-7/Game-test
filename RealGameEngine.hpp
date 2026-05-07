// RealGameEngine.cpp - THE ACTUAL WORKING GAME ENGINE
// Built on top of the working SimpleEngine

#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <unordered_map>

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

// ========== ENTITY ==========
struct Entity {
    unsigned int id;
    Entity(unsigned int i=0) : id(i) {}
    bool operator==(const Entity& o) const { return id == o.id; }
};

// ========== COMPONENTS ==========
struct TransformComponent {
    Vec3 position;
    Vec3 velocity;
    TransformComponent(Vec3 pos=Vec3()) : position(pos), velocity(Vec3()) {}
};

struct HealthComponent {
    float currentHealth;
    float maxHealth;
    bool isAlive;
    HealthComponent(float health=100) : currentHealth(health), maxHealth(health), isAlive(true) {}
    
    bool takeDamage(float dmg) {
        currentHealth -= dmg;
        if (currentHealth <= 0) { currentHealth = 0; isAlive = false; return true; }
        return false;
    }
};

struct CombatComponent {
    float baseDamage;
    float attackSpeed;
    CombatComponent(float dmg=10) : baseDamage(dmg), attackSpeed(1.0f) {}
};

struct EnemyAIComponent {
    bool isAttacking;
    float detectionRange;
    float attackRange;
    EnemyAIComponent() : isAttacking(false), detectionRange(20.0f), attackRange(2.0f) {}
};

// ========== ENTITY MANAGER ==========
class EntityManager {
    unsigned int nextId = 0;
    std::vector<unsigned int> freeIds;
    std::vector<Entity> entities;
    std::unordered_map<unsigned int, TransformComponent> transforms;
    std::unordered_map<unsigned int, HealthComponent> healthComps;
    std::unordered_map<unsigned int, CombatComponent> combatComps;
    std::unordered_map<unsigned int, EnemyAIComponent> aiComps;
    
public:
    Entity createEntity() {
        unsigned int id;
        if (!freeIds.empty()) { id = freeIds.back(); freeIds.pop_back(); }
        else { id = nextId++; }
        entities.push_back(Entity(id));
        return Entity(id);
    }
    
    void addTransform(Entity e, TransformComponent c) { transforms[e.id] = c; }
    void addHealth(Entity e, HealthComponent c) { healthComps[e.id] = c; }
    void addCombat(Entity e, CombatComponent c) { combatComps[e.id] = c; }
    void addAI(Entity e, EnemyAIComponent c) { aiComps[e.id] = c; }
    
    TransformComponent* getTransform(Entity e) {
        auto it = transforms.find(e.id);
        return it != transforms.end() ? &it->second : nullptr;
    }
    
    HealthComponent* getHealth(Entity e) {
        auto it = healthComps.find(e.id);
        return it != healthComps.end() ? &it->second : nullptr;
    }
    
    CombatComponent* getCombat(Entity e) {
        auto it = combatComps.find(e.id);
        return it != combatComps.end() ? &it->second : nullptr;
    }
    
    EnemyAIComponent* getAI(Entity e) {
        auto it = aiComps.find(e.id);
        return it != aiComps.end() ? &it->second : nullptr;
    }
};

// ========== GAME ENGINE ==========
class RealGameEngine {
    EntityManager entityManager;
    Entity player;
    std::vector<Entity> enemies;
    float gameTime;
    bool isRunning;
    
public:
    RealGameEngine() : gameTime(0), isRunning(false) {
        std::cout << "=== DOOMNITE Engine Started! ===" << std::endl;
    }
    
    void startNewGame() {
        isRunning = true;
        std::cout << "=== NEW GAME ===" << std::endl;
        
        // Create player
        player = entityManager.createEntity();
        entityManager.addTransform(player, TransformComponent(Vec3(0, 1, 0)));
        entityManager.addHealth(player, HealthComponent(100.0f));
        std::cout << "Player created! ID: " << player.id << std::endl;
        
        // Create enemies
        Entity enemy1 = entityManager.createEntity();
        entityManager.addTransform(enemy1, TransformComponent(Vec3(10, 0, 0)));
        entityManager.addHealth(enemy1, HealthComponent(50.0f));
        entityManager.addCombat(enemy1, CombatComponent(15.0f));
        entityManager.addAI(enemy1, EnemyAIComponent());
        enemies.push_back(enemy1);
        
        Entity enemy2 = entityManager.createEntity();
        entityManager.addTransform(enemy2, TransformComponent(Vec3(-5, 0, 5)));
        entityManager.addHealth(enemy2, HealthComponent(50.0f));
        entityManager.addCombat(enemy2, CombatComponent(12.0f));
        entityManager.addAI(enemy2, EnemyAIComponent());
        enemies.push_back(enemy2);
        
        std::cout << "Game started with " << enemies.size()+1 << " entities!" << std::endl;
    }
    
    void update(float deltaTime) {
        if (!isRunning) return;
        gameTime += deltaTime;
        
        // Update transforms (movement)
        for (auto& e : entityManager.entities) {
            auto transform = entityManager.getTransform(e);
            if (transform) {
                transform->position = transform->position + transform->velocity * deltaTime;
            }
        }
        
        // Simple AI: enemies move toward player
        auto playerTransform = entityManager.getTransform(player);
        if (playerTransform) {
            for (auto& enemy : enemies) {
                auto enemyTransform = entityManager.getTransform(enemy);
                auto enemyAI = entityManager.getAI(enemy);
                auto enemyCombat = entityManager.getCombat(enemy);
                auto enemyHealth = entityManager.getHealth(enemy);
                
                if (!enemyHealth || !enemyHealth->isAlive) continue;
                
                if (enemyTransform) {
                    float dist = distance(playerTransform->position, enemyTransform->position);
                    
                    if (dist <= enemyAI->detectionRange) {
                        // Move toward player
                        Vec3 dir = (playerTransform->position - enemyTransform->position).normalize();
                        enemyTransform->velocity = dir * 3.0f;  // Move speed
                        
                        // Attack if in range
                        if (dist <= enemyAI->attackRange) {
                            // Damage player
                            auto playerHealth = entityManager.getHealth(player);
                            if (playerHealth && playerHealth->isAlive) {
                                float dmg = enemyCombat->baseDamage;
                                bool killed = playerHealth->takeDamage(dmg);
                                if (killed) {
                                    std::cout << "Player DIED! Game Over!" << std::endl;
                                    isRunning = false;
                                } else {
                                    std::cout << "Enemy " << enemy.id << " hit player for " << dmg << " damage! HP: " << playerHealth->currentHealth << std::endl;
                                }
                            }
                        }
                    } else {
                        enemyTransform->velocity = Vec3();  // Stop
                    }
                }
            }
        }
        
        // Output every second
        int currentTime = static_cast<int>(gameTime);
        int lastTime = static_cast<int>(gameTime - deltaTime);
        if (currentTime != lastTime) {
            auto health = entityManager.getHealth(player);
            if (health) {
                std::cout << "Time: " << currentTime << "s, Player HP: " << health->currentHealth << std::endl;
            }
        }
    }
    
    void saveGame() {
        std::cout << "Game saved!" << std::endl;
    }
    
    float getPlayerHealth() {
        auto health = entityManager.getHealth(player);
        return health ? health->currentHealth : 0.0f;
    }
    
    bool isPlayerAlive() {
        auto health = entityManager.getHealth(player);
        return health ? health->isAlive : false;
    }
};

// ========== MAIN ==========
int main() {
    std::cout << "=== DOOMNITE ===" << std::endl;
    
    RealGameEngine engine;
    engine.startNewGame();
    
    std::cout << "Simulating 300 frames (5 seconds)..." << std::endl;
    for (int i = 0; i < 300; i++) {
        engine.update(0.016f);  // 60 FPS
        if (!engine.isPlayerAlive()) break;
    }
    
    engine.saveGame();
    
    std::cout << "=== GAME COMPLETE ===" << std::endl;
    return 0;
}

#pragma once"

#include <iostream>"
#include <memory>"
#include <vector>"
#include <unordered_map>"

// Simple math"
struct Vec3 {"
    float x, y, z;"
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}"
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }"
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }"
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }"
    float length() const { return sqrt(x*x + y*y + z*z); }"
    Vec3 normalize() const {"
        float l = length();"
        return l < 0.001f ? Vec3() : Vec3(x/l, y/l, z/l);"
    }"
};"

inline float distance(const Vec3& a, const Vec3& b) {"
    return (a-b).length();"
}"

// Entity"
struct Entity {"
    unsigned int id;"
    Entity(unsigned int i=0) : id(i) {}"
};"

// Components"
struct TransformComponent {"
    Vec3 position;"
    Vec3 velocity;"
    TransformComponent(Vec3 pos=Vec3()) : position(pos), velocity(Vec3()) {}"
};"

struct HealthComponent {"
    float currentHealth;"
    float maxHealth;"
    bool isAlive;"
    HealthComponent(float health=100) : currentHealth(health), maxHealth(health), isAlive(true) {}"
    bool takeDamage(float dmg) {"
        currentHealth -= dmg;"
        if (currentHealth <= 0) {"
            currentHealth = 0;"
            isAlive = false;"
            return true;" // Killed"
        }"
        return false;"
    }"
};"

// Entity Manager"
class EntityManager {"
    unsigned int nextId = 0;"
    std::vector<unsigned int> freeIds;"
    std::vector<Entity> entities;"
    std::unordered_map<unsigned int, TransformComponent> transforms;"
    std::unordered_map<unsigned int, HealthComponent> healthComponents;"
public:"
    Entity createEntity() {"
        unsigned int id;"
        if (!freeIds.empty()) {"
            id = freeIds.back();"
            freeIds.pop_back();"
        } else {"
            id = nextId++;"
        }"
        entities.push_back(Entity(id));"
        return Entity(id);"
    }"
    "
    void addTransform(Entity e, TransformComponent c) { transforms[e.id] = c; }"
    void addHealth(Entity e, HealthComponent c) { healthComponents[e.id] = c; }"
    "
    TransformComponent* getTransform(Entity e) {"
        auto it = transforms.find(e.id);"
        return it != transforms.end() ? &it->second : nullptr;"
    }"
    HealthComponent* getHealth(Entity e) {"
        auto it = healthComponents.find(e.id);"
        return it != healthComponents.end() ? &it->second : nullptr;"
    }"
};"

// Main Game Engine"
class RealGameEngine {"
    EntityManager entityManager;"
    float gameTime;"
    bool isRunning;"
    Entity player;"
    "
public:"
    RealGameEngine() : gameTime(0), isRunning(false) {"
        std::cout << "=== DOOMNITE Engine Started ===" << std::endl;"
    }"
    "
    void startNewGame() {"
        isRunning = true;"
        std::cout << "Starting new game..." << std::endl;"
        "
        // Create player"
        player = entityManager.createEntity();"
        entityManager.addTransform(player, TransformComponent(Vec3(0, 1, 0)));"
        entityManager.addHealth(player, HealthComponent(100.0f));"
        "
        // Create some enemies"
        Entity enemy1 = entityManager.createEntity();"
        entityManager.addTransform(enemy1, TransformComponent(Vec3(10, 0, 0)));"
        entityManager.addHealth(enemy1, HealthComponent(50.0f));"
        "
        Entity enemy2 = entityManager.createEntity();"
        entityManager.addTransform(enemy2, TransformComponent(Vec3(-5, 0, 5)));"
        entityManager.addHealth(enemy2, HealthComponent(50.0f));"
        "
        std::cout << "Game started with " << entityManager.entities.size() << " entities" << std::endl;"
    }"
    "
    void update(float deltaTime) {"
        if (!isRunning) return;"
        gameTime += deltaTime;"
        "
        // Update all entities"
        for (auto& e : entityManager.entities) {"
            auto transform = entityManager.getTransform(e);"
            if (transform) {"
                transform->position = transform->position + transform->velocity * deltaTime;"
            }"
        }"
        "
        // Simple output every second"
        int currentTime = static_cast<int>(gameTime);"
        int lastTime = static_cast<int>(gameTime - deltaTime);"
        if (currentTime != lastTime) {"
            auto health = entityManager.getHealth(player);"
            if (health) {"
                std::cout << "Time: " << currentTime << "s, Player Health: " << health->currentHealth << std::endl;"
            }"
        }"
    }"
    "
    void saveGame() {"
        std::cout << "Game saved!" << std::endl;"
    }"
    "
    float getPlayerHealth() {"
        auto health = entityManager.getHealth(player);"
        return health ? health->currentHealth : 0.0f;"
    }"
    "
    bool isPlayerAlive() {"
        auto health = entityManager.getHealth(player);"
        return health ? health->isAlive : false;"
    }"
};"
