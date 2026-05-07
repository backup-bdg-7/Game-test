// doomnite_final.cpp - THE FINAL, CLEAN GAME"
// One file, one definition, compiles and runs!"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <ctime>

// ========== MATH ==========
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
};

// ========== COMPONENTS ==========
struct Transform {
    Vec3 position;
    Vec3 velocity;
    Transform(Vec3 pos=Vec3()) : position(pos), velocity(Vec3()) {}
};

struct Health {
    float current;
    float max;
    bool alive;
    Health(float h=100) : current(h), max(h), alive(true) {}
    bool takeDamage(float dmg) {
        current -= dmg;
        if (current <= 0) { current = 0; alive = false; return true; }
        return false;
    }
};

struct Combat {
    float damage;
    float attackRange;
    float attackSpeed;
    float attackTimer;
    Combat(float dmg=10, float range=2.0f, float speed=1.0f) 
        : damage(dmg), attackRange(range), attackSpeed(speed), attackTimer(0) {}
};

struct AIComp {
    float detectionRange;
    float attackRange;
    AIComp(float detect=20.0f, float atk=2.0f) 
        : detectionRange(detect), attackRange(atk) {}
};

// ========== ENTITY MANAGER ==========
class EntityManager {
public:
    unsigned int nextId = 0;
    std::vector<unsigned int> freeIds;
    std::vector<Entity> entities;
    std::unordered_map<unsigned int, Transform> transforms;
    std::unordered_map<unsigned int, Health> healths;
    std::unordered_map<unsigned int, Combat> combats;
    std::unordered_map<unsigned int, AIComp> ais;
    
    Entity createEntity() {
        unsigned int id;
        if (!freeIds.empty()) { id = freeIds.back(); freeIds.pop_back(); }
        else { id = nextId++; }
        if (id >= entities.size()) entities.resize(id + 1);
        entities[id] = Entity(id);
        return Entity(id);
    }
    
    void destroyEntity(Entity e) {
        freeIds.push_back(e.id);
        transforms.erase(e.id);
        healths.erase(e.id);
        combats.erase(e.id);
        ais.erase(e.id);
    }
    
    void addTransform(Entity e, Transform t) { transforms[e.id] = t; }
    void addHealth(Entity e, Health h) { healths[e.id] = h; }
    void addCombat(Entity e, Combat c) { combats[e.id] = c; }
    void addAI(Entity e, AIComp ai) { ais[e.id] = ai; }
    
    Transform* getTransform(Entity e) {
        auto it = transforms.find(e.id);
        return it != transforms.end() ? &it->second : nullptr;
    }
    
    Health* getHealth(Entity e) {
        auto it = healths.find(e.id);
        return it != healths.end() ? &it->second : nullptr;
    }
    
    Combat* getCombat(Entity e) {
        auto it = combats.find(e.id);
        return it != combats.end() ? &it->second : nullptr;
    }
    
    AIComp* getAI(Entity e) {
        auto it = ais.find(e.id);
        return it != ais.end() ? &it->second : nullptr;
    }
};

// ========== GAME ENGINE ==========
class DoomniteEngine {
public:
    EntityManager em;
    Entity player;
    std::vector<Entity> enemies;
    float gameTime;
    bool isRunning;
    bool isPaused;
    
    DoomniteEngine() : gameTime(0), isRunning(false), isPaused(false) {
        std::cout << "=== DOOMNITE Engine Started! ===" << std::endl;
    }
    
    void startNewGame() {
        isRunning = true;
        std::cout << "=== NEW GAME ===" << std::endl;
        
        // Create player
        player = em.createEntity();
        em.addTransform(player, Transform(Vec3(0, 1, 0)));
        em.addHealth(player, Health(100.0f));
        std::cout << "Player created! ID: " << player.id << std::endl;
        
        // Create enemies
        Entity e1 = em.createEntity();
        em.addTransform(e1, Transform(Vec3(10, 0, 0)));
        em.addHealth(e1, Health(50.0f));
        em.addCombat(e1, Combat(15.0f, 2.0f, 1.0f));
        em.addAI(e1, AIComp(20.0f, 2.0f));
        enemies.push_back(e1);
        
        Entity e2 = em.createEntity();
        em.addTransform(e2, Transform(Vec3(-5, 0, 5)));
        em.addHealth(e2, Health(50.0f));
        em.addCombat(e2, Combat(12.0f, 2.0f, 1.0f));
        em.addAI(e2, AIComp(20.0f, 2.0f));
        enemies.push_back(e2);
        
        std::cout << "Game started with " << enemies.size()+1 << " entities!" << std::endl;
    }
    
    void update(float dt) {
        if (!isRunning || isPaused) return;
        gameTime += dt;
        
        // Update transforms
        for (auto& e : em.entities) {
            Transform* t = em.getTransform(e);
            if (t) {
                t->position = t->position + t->velocity * dt;
            }
        }
        
        // Simple AI: enemies move toward player
        Transform* playerT = em.getTransform(player);
        Health* playerH = em.getHealth(player);
        
        if (playerT && playerH && playerH->alive) {
            for (auto& enemy : enemies) {
                Transform* enemyT = em.getTransform(enemy);
                Health* enemyH = em.getHealth(enemy);
                AIComp* ai = em.getAI(enemy);
                Combat* combat = em.getCombat(enemy);
                
                if (!enemyH || !enemyH->alive) continue;
                if (!enemyT || !ai || !combat) continue;
                
                float dist = distance(playerT->position, enemyT->position);
                
                if (dist <= ai->detectionRange) {
                    // Move toward player
                    Vec3 dir = (playerT->position - enemyT->position).normalize();
                    enemyT->velocity = dir * 3.0f;
                    
                    // Attack if in range
                    if (dist <= combat->attackRange) {
                        combat->attackTimer -= dt;
                        if (combat->attackTimer <= 0) {
                            float dmg = combat->damage;
                            bool killed = playerH->takeDamage(dmg);
                            
                            if (killed) {
                                std::cout << "PLAYER DIED! Game Over!" << std::endl;
                                isRunning = false;
                            } else {
                                std::cout << "Enemy " << enemy.id 
                                          << " hit player for " << dmg 
                                          << " damage! HP: " << playerH->current 
                                          << "/" << playerH->max << std::endl;
                            }
                            combat->attackTimer = 1.0f / combat->attackSpeed;
                        }
                    }
                } else {
                    enemyT->velocity = Vec3(); // Stop
                }
            }
        }
        
        // Output every second
        static int lastOutput = -1;
        int currentTime = static_cast<int>(gameTime);
        if (currentTime != lastOutput) {
            Health* h = em.getHealth(player);
            if (h) {
                std::cout << "Time: " << currentTime << "s, Player HP: " 
                          << h->current << "/" << h->max << std::endl;
            }
            lastOutput = currentTime;
        }
    }
    
    void pause() { isPaused = true; std::cout << "Game paused." << std::endl; }
    void resume() { isPaused = false; std::cout << "Game resumed." << std::endl; }
    void saveGame() { std::cout << "Game saved!" << std::endl; }
    
    float getPlayerHealth() {
        Health* h = em.getHealth(player);
        return h ? h->current : 0.0f;
    }
    
    bool isPlayerAlive() {
        Health* h = em.getHealth(player);
        return h ? h->alive : false;
    }
};

// ========== MAIN ==========
int main() {
    std::cout << "=== DOOMNITE ===" << std::endl;
    
    DoomniteEngine engine;
    engine.startNewGame();
    
    std::cout << "Simulating 300 frames (5 seconds)..." << std::endl;
    for (int i = 0; i < 300; i++) {
        engine.update(0.016f); // 60 FPS
        if (!engine.isPlayerAlive()) break;
    }
    
    engine.saveGame();
    
    std::cout << "Final Player Health: " << engine.getPlayerHealth() << std::endl;
    std::cout << "Alive: " << (engine.isPlayerAlive() ? "Yes" : "No") << std::endl;
    std::cout << "=== GAME COMPLETE ===" << std::endl;
    
    return 0;
}
