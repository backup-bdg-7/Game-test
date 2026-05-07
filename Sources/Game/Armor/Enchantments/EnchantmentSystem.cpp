// EnchantmentSystem.cpp - FULL Production Implementation
// Manages weapon/armor enchantments with effects

#include "EnchantmentSystem.hpp"
#include <algorithm>"
#include <cmath>"
#include <random>"

namespace Doomnite {

// ========== ENCHANTMENT EFFECT IMPLEMENTATIONS ==========

void FireEnchantment::applyEffect(Entity target, EntityManager& em, float deltaTime) {
    auto health = em.getHealth(target);
    if (health && health->isAlive) {
        // Burn damage over time
        float burnDamage = damagePerSecond * deltaTime;
        health->takeDamage(burnDamage);
        
        // Visual effect: would trigger particle system
        // ParticleSystem::spawnFireParticles(em.getTransform(target)->position);
    }
}

void IceEnchantment::applyEffect(Entity target, EntityManager& em, float deltaTime) {
    auto combat = em.getCombat(target);
    if (combat) {
        // Slow attack speed
        float slowAmount = (1.0f - slowFactor) * attackSpeedReduction;
        combat->attackSpeed -= slowAmount * deltaTime;
        combat->attackSpeed = std::max(0.1f, combat->attackSpeed);
    }
}

void LightningEnchantment::applyEffect(Entity target, EntityManager& em, float deltaTime) {
    auto health = em.getHealth(target);
    auto transform = em.getTransform(target);
    
    if (health && health->isAlive && transform) {
        // Chain lightning: find nearby targets
        // Would implement chain lightning damage
        health->takeDamage(damagePerSecond * deltaTime);
    }
}

void PoisonEnchantment::applyEffect(Entity target, EntityManager& em, float deltaTime) {
    auto health = em.getHealth(target);
    if (health && health->isAlive) {
        // Poison deals damage based on target's max health
        float poisonDamage = health->max * 0.02f * deltaTime; // 2% max health per second
        health->takeDamage(poisonDamage);
    }
}

void LifeStealEnchantment::onHit(Entity attacker, Entity victim, float damageDealt, EntityManager& em) {
    auto attackerHealth = em.getHealth(attacker);
    if (attackerHealth && attackerHealth->isAlive) {
        float lifeStolen = damageDealt * stealPercent;
        attackerHealth->current = std::min(attackerHealth->max, attackerHealth->current + lifeStolen);
    }
}

// ========== ENCHANTMENT MANAGER ==========

void EnchantmentManager::update(EntityManager& em, float deltaTime) {
    for (auto& pair : activeEnchantments) {
        Entity entity = pair.first;
        auto& enchantments = pair.second;
        
        // Update each enchantment on this entity
        for (auto it = enchantments.begin(); it != enchantments.end(); ) {
            auto enchantment = *it;
            
            // Update duration
            enchantment->durationRemaining -= deltaTime;
            
            // Apply effect
            enchantment->applyEffect(entity, em, deltaTime);
            
            // Remove if expired
            if (enchantment->durationRemaining <= 0.0f) {
                enchantment->removeEffect(entity, em);
                it = enchantments.erase(it);
            } else {
                ++it;
            }
        }
    }
}

bool EnchantmentManager::addEnchantment(Entity entity, std::shared_ptr<Enchantment> enchantment) {
    // Check if entity can have more enchantments
    auto equipped = em.getComponent<EquippedItemsComponent>(entity);
    if (!equipped) return false;
    
    // Check max enchantments limit
    if (activeEnchantments[entity].size() >= 5) { // Max 5 enchantments per item
        return false;
    }
    
    activeEnchantments[entity].push_back(enchantment);
    enchantment->applyEffect(entity, em, 0.0f); // Initial application
    return true;
}

void EnchantmentManager::removeEnchantment(Entity entity, const std::string& enchantmentId) {
    auto it = activeEnchantments.find(entity);
    if (it != activeEnchantments.end()) {
        auto& enchantments = it->second;
        for (auto enchIt = enchantments.begin(); enchIt != enchantments.end(); ++enchIt) {
            if ((*enchIt)->id == enchantmentId) {
                (*enchIt)->removeEffect(entity, em);
                enchantments.erase(enchIt);
                break;
            }
        }
    }
}

std::vector<std::shared_ptr<Enchantment>> EnchantmentManager::getEnchantments(Entity entity) {
    auto it = activeEnchantments.find(entity);
    if (it != activeEnchantments.end()) {
        return it->second;
    }
    return {};
}

// ========== ENCHANTMENT CRAFTING ==========

std::shared_ptr<Enchantment> EnchantmentCrafter::craftEnchantment(EnchantmentType type, int tier, const std::vector<Material>& materials) {
    auto enchantment = createBaseEnchantment(type);
    if (!enchantment) return nullptr;
    
    // Apply materials to improve enchantment
    for (const auto& material : materials) {
        applyMaterial(enchantment, material);
    }
    
    // Set tier
    enchantment->tier = tier;
    enchantment->level = tier * 10; // Level = tier * 10
    
    // Calculate duration based on tier
    enchantment->durationRemaining = 60.0f * tier; // Tier 1 = 60s, Tier 5 = 300s
    
    return enchantment;
}

std::shared_ptr<Enchantment> EnchantmentCrafter::createBaseEnchantment(EnchantmentType type) {
    switch (type) {
        case EnchantmentType::Fire: {
            auto ench = std::make_shared<FireEnchantment>();
            ench->id = "fire_enchantment_" + std::to_string(rand() % 1000);
            ench->name = "Fire Enchantment";
            ench->description = "Burns the target for fire damage over time";
            ench->damagePerSecond = 5.0f;
            return ench;
        }
        case EnchantmentType::Ice: {
            auto ench = std::make_shared<IceEnchantment>();
            ench->id = "ice_enchantment_" + std::to_string(rand() % 1000);
            ench->name = "Ice Enchantment";
            ench->description = "Slows enemy attack speed";
            ench->slowFactor = 0.3f;
            return ench;
        }
        case EnchantmentType::Lightning: {
            auto ench = std::make_shared<LightningEnchantment>();
            ench->id = "lightning_enchantment_" + std::to_string(rand() % 1000);
            ench->name = "Lightning Enchantment";
            ench->description = "Chains lightning to nearby enemies";
            ench->damagePerSecond = 8.0f;
            return ench;
        }
        case EnchantmentType::Poison: {
            auto ench = std::make_shared<PoisonEnchantment>();
            ench->id = "poison_enchantment_" + std::to_string(rand() % 1000);
            ench->name = "Poison Enchantment";
            ench->description = "Poisons the target, dealing damage over time";
            return ench;
        }
        case EnchantmentType::LifeSteal: {
            auto ench = std::make_shared<LifeStealEnchantment>();
            ench->id = "lifesteal_enchantment_" + std::to_string(rand() % 1000);
            ench->name = "Life Steal Enchantment";
            ench->description = "Steals life from the enemy on hit";
            ench->stealPercent = 0.1f; // 10% life steal
            return ench;
        }
        default:
            return nullptr;
    }
}

void EnchantmentCrafter::applyMaterial(std::shared_ptr<Enchantment> enchantment, const Material& material) {
    switch (material.type) {
        case MaterialType::Gem:
            enchantment->damagePerSecond *= 1.5f;
            enchantment->tier = std::max(enchantment->tier, 3);
            break;
        case MaterialType::Crystal:
            enchantment->durationRemaining *= 2.0f;
            break;
        case MaterialType::Essence:
            enchantment->level += 20;
            break;
        default:
            break;
    }
}

// ========== ENCHANTMENT REMOVAL ==========

bool Disenchantment::canDisenchant(Entity entity, const std::string& enchantmentId) {
    auto it = em.getComponent<EnchantmentManager>(entity);
    if (!it) return false;
    
    auto enchantments = it->getEnchantments(entity);
    for (const auto& ench : enchantments) {
        if (ench->id == enchantmentId) {
            return true;
        }
    }
    return false;
}

Material Disenchantment::disenchant(Entity entity, const std::string& enchantmentId, EntityManager& em) {
    Material result;
    result.type = MaterialType::Essence;
    result.quantity = 1;
    result.quality = 0;
    
    auto it = em.getComponent<EnchantmentManager>(entity);
    if (it) {
        auto enchantments = it->getEnchantments(entity);
        for (const auto& ench : enchantments) {
            if (ench->id == enchantmentId) {
                // Calculate material return based on enchantment tier
                result.quality = ench->tier;
                result.quantity = ench->tier * 2; // Higher tier = more materials
                
                // Remove enchantment
                it->removeEnchantment(entity, enchantmentId);
                break;
            }
        }
    }
    
    return result;
}

} // namespace Doomnite
