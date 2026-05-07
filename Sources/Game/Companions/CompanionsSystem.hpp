// CompanionsSystem.hpp - C++ Animal Companions System for Doomnite
// Dragons, Wolves, Bears, Birds with combat abilities and evolution

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Companion Type

enum class CompanionType {
    Dragon,
    Wolf,
    Bear,
    Bird
};

// MARK: - Companion Element

enum class CompanionElement {
    Fire,
    Ice,
    Lightning,
    Poison,
    None
};

// MARK: - Evolution Stage

enum class EvolutionStage {
    Baby,      // Weak, small
    Juvenile,   // Medium strength
    Adult,      // Full strength
    Alpha       // Maxed stats, special abilities
};

// MARK: - Companion Ability

struct CompanionAbility {
    std::string id;
    std::string name;
    std::string description;
    float cooldown;
    float currentCooldown;
    float damage;
    float radius;      // For AOE abilities
    bool isUnlocked;
    
    CompanionAbility(const std::string& abilityId, const std::string& abilityName,
                   const std::string& desc, float cd, float dmg = 10.0f,
                   float rad = 0.0f, bool unlocked = false)
        : id(abilityId), name(abilityName), description(desc), cooldown(cd),
          currentCooldown(0.0f), damage(dmg), radius(rad), isUnlocked(unlocked) {}
    
    bool canUse() const {
        return currentCooldown <= 0.0f && isUnlocked;
    }
    
    void use() {
        currentCooldown = cooldown;
    }
    
    void update(float deltaTime) {
        if (currentCooldown > 0.0f) {
            currentCooldown -= deltaTime;
        }
    }
};

// MARK: - Companion Definition

struct CompanionDefinition {
    std::string id;
    std::string name;
    CompanionType type;
    CompanionElement element;
    EvolutionStage baseStage;
    
    float baseHealth;
    float baseDamage;
    float baseSpeed;
    int baseLevel;
    
    std::vector<std::shared_ptr<CompanionAbility>> abilities;
    std::vector<std::string> evolutionRequirements;  // Item IDs needed
    std::string evolutionToId;  // Next evolution form
    
    CompanionDefinition(const std::string& compId, const std::string& compName,
                        CompanionType compType, CompanionElement elem = CompanionElement::None,
                        EvolutionStage stage = EvolutionStage::Baby,
                        float health = 50.0f, float damage = 10.0f, float speed = 5.0f)
        : id(compId), name(compName), type(compType), element(elem), baseStage(stage),
          baseHealth(health), baseDamage(damage), baseSpeed(speed), baseLevel(1) {}
    
    std::shared_ptr<CompanionAbility> getAbility(const std::string& abilityId) const {
        for (const auto& ability : abilities) {
            if (ability->id == abilityId) {
                return ability;
            }
        }
        return nullptr;
    }
};

// MARK: - Companion Component (ECS)

struct CompanionComponent : public Component {
    std::string definitionId;
    CompanionType type;
    CompanionElement element;
    EvolutionStage stage;
    int level;
    float experience;
    float experienceToNextLevel;
    
    float currentHealth;
    float maxHealth;
    float damage;
    float speed;
    float attackRange;
    
    std::vector<std::shared_ptr<CompanionAbility>> unlockedAbilities;
    Entity ownerEntity;  // Player who owns this companion
    
    bool isFollowing;
    bool isAttacking;
    Entity targetEntity;
    float timeSinceLastAttack;
    
    CompanionComponent(const std::string& defId = "", CompanionType compType = CompanionType::Wolf,
                       CompanionElement elem = CompanionElement::None,
                       EvolutionStage evoStage = EvolutionStage::Adult)
        : definitionId(defId), type(compType), element(elem), stage(evoStage),
          level(1), experience(0.0f), experienceToNextLevel(100.0f),
          currentHealth(100.0f), maxHealth(100.0f), damage(10.0f), speed(5.0f),
          attackRange(3.0f), isFollowing(true), isAttacking(false),
          timeSinceLastAttack(0.0f) {}
    
    float getHealthPercentage() const {
        return currentHealth / maxHealth;
    }
    
    bool isAlive() const {
        return currentHealth > 0.0f;
    }
    
    void addExperience(float xp) {
        experience += xp;
        while (experience >= experienceToNextLevel && level < 50) {
            levelUp();
        }
    }
    
    void levelUp() {
        level++;
        experience -= experienceToNextLevel;
        experienceToNextLevel *= 1.2f;  // Each level needs 20% more XP
        
        // Increase stats
        maxHealth *= 1.1f;
        damage *= 1.05f;
        speed *= 1.02f;
        
        currentHealth = maxHealth;  // Heal on level up
    }
    
    bool evolve(EvolutionStage newStage) {
        if (newStage <= stage) return false;
        if (level < getRequiredLevelForStage(newStage)) return false;
        
        stage = newStage;
        
        // Boost stats based on evolution
        float multiplier = getEvolutionMultiplier(newStage);
        maxHealth *= multiplier;
        damage *= multiplier;
        speed *= (1.0f + (multiplier - 1.0f) * 0.5f);
        
        currentHealth = maxHealth;
        return true;
    }
    
    float getEvolutionMultiplier(EvolutionStage stage) const {
        switch (stage) {
            case EvolutionStage::Baby: return 0.5f;
            case EvolutionStage::Juvenile: return 0.8f;
            case EvolutionStage::Adult: return 1.0f;
            case EvolutionStage::Alpha: return 2.0f;
        }
        return 1.0f;
    }
    
    int getRequiredLevelForStage(EvolutionStage stage) const {
        switch (stage) {
            case EvolutionStage::Baby: return 1;
            case EvolutionStage::Juvenile: return 10;
            case EvolutionStage::Adult: return 25;
            case EvolutionStage::Alpha: return 40;
        }
        return 1;
    }
    
    void update(float deltaTime) {
        timeSinceLastAttack += deltaTime;
        
        // Update ability cooldowns
        for (auto& ability : unlockedAbilities) {
            ability->update(deltaTime);
        }
    }
    
    bool useAbility(const std::string& abilityId) {
        for (auto& ability : unlockedAbilities) {
            if (ability->id == abilityId && ability->canUse()) {
                ability->use();
                // Would trigger ability effects here
                return true;
            }
        }
        return false;
    }
};

// MARK: - Predefined Companions

namespace Companions {
    
    // MARK: Dragons
    static CompanionDefinition FireDragon("companion_fire_dragon", "Fire Dragon",
                                           CompanionType::Dragon, CompanionElement::Fire,
                                           EvolutionStage::Baby, 200.0f, 30.0f, 8.0f);
    
    static CompanionDefinition IceDragon("companion_ice_dragon", "Ice Dragon",
                                          CompanionType::Dragon, CompanionElement::Ice,
                                          EvolutionStage::Baby, 180.0f, 25.0f, 7.0f);
    
    static CompanionDefinition LightningDragon("companion_lightning_dragon", "Lightning Dragon",
                                               CompanionType::Dragon, CompanionElement::Lightning,
                                               EvolutionStage::Baby, 160.0f, 35.0f, 9.0f);
    
    // MARK: Wolves
    static CompanionDefinition DireWolf("companion_dire_wolf", "Dire Wolf",
                                         CompanionType::Wolf, CompanionElement::None,
                                         EvolutionStage::Juvenile, 80.0f, 15.0f, 12.0f);
    
    static CompanionDefinition IceWolf("companion_ice_wolf", "Ice Wolf",
                                        CompanionType::Wolf, CompanionElement::Ice,
                                        EvolutionStage::Juvenile, 90.0f, 18.0f, 11.0f);
    
    // MARK: Bears
    static CompanionDefinition GrizzlyBear("companion_grizzly", "Grizzly Bear",
                                          CompanionType::Bear, CompanionElement::None,
                                          EvolutionStage::Adult, 150.0f, 20.0f, 5.0f);
    
    static CompanionDefinition CaveBear("companion_cave_bear", "Cave Bear",
                                         CompanionType::Bear, CompanionElement::None,
                                         EvolutionStage::Adult, 200.0f, 25.0f, 4.0f);
    
    // MARK: Birds
    static CompanionDefinition Eagle("companion_eagle", "Eagle",
                                      CompanionType::Bird, CompanionElement::None,
                                      EvolutionStage::Adult, 40.0f, 8.0f, 20.0f);
    
    static std::vector<std::shared_ptr<CompanionDefinition>> AllCompanions = {
        std::make_shared<CompanionDefinition>(FireDragon),
        std::make_shared<CompanionDefinition>(IceDragon),
        std::make_shared<CompanionDefinition>(LightningDragon),
        std::make_shared<CompanionDefinition>(DireWolf),
        std::make_shared<CompanionDefinition>(IceWolf),
        std::make_shared<CompanionDefinition>(GrizzlyBear),
        std::make_shared<CompanionDefinition>(CaveBear),
        std::make_shared<CompanionDefinition>(Eagle)
    };
    
    static std::shared_ptr<CompanionDefinition> getCompanionById(const std::string& id) {
        for (const auto& comp : AllCompanions) {
            if (comp->id == id) return comp;
        }
        return nullptr;
    }
}

// MARK: - Companion System

class CompanionSystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        auto companions = entities.componentStorage.getAllComponents<CompanionComponent>();
        
        for (auto& pair : companions) {
            Entity entity = pair.first;
            auto companion = pair.second;
            
            if (!companion->isAlive()) continue;
            
            companion->update(deltaTime);
            
            // AI behavior based on type
            switch (companion->type) {
                case CompanionType::Dragon:
                    updateDragon(entity, companion, entities, deltaTime);
                    break;
                case CompanionType::Wolf:
                    updateWolf(entity, companion, entities, deltaTime);
                    break;
                case CompanionType::Bear:
                    updateBear(entity, companion, entities, deltaTime);
                    break;
                case CompanionType::Bird:
                    updateBird(entity, companion, entities, deltaTime);
                    break;
            }
            
            // Update back in storage
            entities.componentStorage.addComponent<CompanionComponent>(entity, companion);
        }
    }
    
private:
    void updateDragon(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                       EntityManager& entities, float deltaTime) {
        // Dragons: fly, breathe fire/ice/lightning, massive damage
        if (companion->isAttacking && companion->timeSinceLastAttack > 2.0f) {
            // Use breath attack
            companion->useAbility("breath_attack");
            companion->timeSinceLastAttack = 0.0f;
        }
    }
    
    void updateWolf(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                    EntityManager& entities, float deltaTime) {
        // Wolves: pack tactics, speed, bleed attacks
        if (companion->isAttacking && companion->timeSinceLastAttack > 1.0f) {
            // Bite attack
            companion->timeSinceLastAttack = 0.0f;
        }
    }
    
    void updateBear(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                    EntityManager& entities, float deltaTime) {
        // Bears: tanking, ground slam, high HP
        if (companion->isAttacking && companion->timeSinceLastAttack > 3.0f) {
            // Ground slam
            companion->useAbility("ground_slam");
            companion->timeSinceLastAttack = 0.0f;
        }
    }
    
    void updateBird(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                    EntityManager& entities, float deltaTime) {
        // Birds: scouting, dropping bombs, revealing secrets
        if (companion->isAttacking && companion->timeSinceLastAttack > 4.0f) {
            // Bomb drop
            companion->useAbility("bomb_drop");
            companion->timeSinceLastAttack = 0.0f;
        }
    }
};

} // namespace Doomnite
