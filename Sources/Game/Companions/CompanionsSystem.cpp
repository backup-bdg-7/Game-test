// CompanionsSystem.cpp - C++ Animal Companions Implementation

#include "CompanionsSystem.hpp"
#include <algorithm>
#include <cmath>

namespace Doomnite {

// MARK: - CompanionComponent Implementation

float CompanionComponent::getHealthPercentage() const {
    if (maxHealth <= 0.0f) return 0.0f;
    return currentHealth / maxHealth;
}

bool CompanionComponent::isAlive() const {
    return currentHealth > 0.0f;
}

void CompanionComponent::addExperience(float xp) {
    if (!isAlive()) return;
    
    experience += xp;
    while (experience >= experienceToNextLevel && level < 50) {
        levelUp();
    }
}

void CompanionComponent::levelUp() {
    level++;
    experience -= experienceToNextLevel;
    experienceToNextLevel *= 1.2f;  // Each level needs 20% more XP
    
    // Increase stats
    maxHealth *= 1.1f;
    damage *= 1.05f;
    speed *= 1.02f;
    
    currentHealth = maxHealth;  // Heal on level up
}

bool CompanionComponent::evolve(EvolutionStage newStage) {
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

float CompanionComponent::getEvolutionMultiplier(EvolutionStage stage) const {
    switch (stage) {
        case EvolutionStage::Baby: return 0.5f;
        case EvolutionStage::Juvenile: return 0.8f;
        case EvolutionStage::Adult: return 1.0f;
        case EvolutionStage::Alpha: return 2.0f;
    }
    return 1.0f;
}

int CompanionComponent::getRequiredLevelForStage(EvolutionStage stage) const {
    switch (stage) {
        case EvolutionStage::Baby: return 1;
        case EvolutionStage::Juvenile: return 10;
        case EvolutionStage::Adult: return 25;
        case EvolutionStage::Alpha: return 40;
    }
    return 1;
}

void CompanionComponent::update(float deltaTime) {
    timeSinceLastAttack += deltaTime;
    
    // Update ability cooldowns
    for (auto& ability : unlockedAbilities) {
        ability->update(deltaTime);
    }
}

bool CompanionComponent::useAbility(const std::string& abilityId) {
    for (auto& ability : unlockedAbilities) {
        if (ability->id == abilityId && ability->canUse()) {
            ability->use();
            // Would trigger ability effects here
            return true;
        }
    }
    return false;
}

// MARK: - CompanionSystem Implementation

void CompanionSystem::update(EntityManager& entities, float deltaTime) {
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

void CompanionSystem::updateDragon(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                                   EntityManager& entities, float deltaTime) {
    // Dragons: fly, breathe fire/ice/lightning, massive damage
    if (companion->isAttacking && companion->timeSinceLastAttack > 2.0f) {
        // Use breath attack
        companion->useAbility("breath_attack");
        companion->timeSinceLastAttack = 0.0f;
    }
}

void CompanionSystem::updateWolf(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                                EntityManager& entities, float deltaTime) {
    // Wolves: pack tactics, speed, bleed attacks
    if (companion->isAttacking && companion->timeSinceLastAttack > 1.0f) {
        // Bite attack
        companion->timeSinceLastAttack = 0.0f;
    }
}

void CompanionSystem::updateBear(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                                EntityManager& entities, float deltaTime) {
    // Bears: tanking, ground slam, high HP
    if (companion->isAttacking && companion->timeSinceLastAttack > 3.0f) {
        // Ground slam
        companion->useAbility("ground_slam");
        companion->timeSinceLastAttack = 0.0f;
    }
}

void CompanionSystem::updateBird(const Entity& entity, std::shared_ptr<CompanionComponent> companion,
                               EntityManager& entities, float deltaTime) {
    // Birds: scouting, dropping bombs, revealing secrets
    if (companion->isAttacking && companion->timeSinceLastAttack > 4.0f) {
        // Bomb drop
        companion->useAbility("bomb_drop");
        companion->timeSinceLastAttack = 0.0f;
    }
}

} // namespace Doomnite
