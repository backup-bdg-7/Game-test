// WeaponUpgradeSystem.cpp - FULL Production Implementation
// Weapon upgrading, modification, stat improvements

#include "WeaponUpgradeSystem.hpp"
#include <algorithm>"
#include <cmath>"
#include <random>"

namespace Doomnite {

// ========== UPGRADE MATERIALS ==========

bool UpgradeMaterial::canUpgrade(const WeaponStats& current, int targetLevel) const {
    if (rarity == MaterialRarity::Common && targetLevel > 3) return false;
    if (rarity == MaterialRarity::Rare && targetLevel > 7) return false;
    if (rarity == MaterialRarity::Epic && targetLevel > 10) return false;
    return quantity >= getRequiredQuantity(targetLevel);
}

int UpgradeMaterial::getRequiredQuantity(int targetLevel) const {
    int base = 1;
    switch (rarity) {
        case MaterialRarity::Common: base = targetLevel; break;
        case MaterialRarity::Uncommon: base = targetLevel * 2; break;
        case MaterialRarity::Rare: base = targetLevel * 3; break;
        case MaterialRarity::Epic: base = targetLevel * 2; break;
        case MaterialRarity::Legendary: base = targetLevel; break;
    }
    return base;
}

// ========== WEAPON STATS UPGRADE ==========

void WeaponStats::upgradeDamage(float amount) {
    baseDamage += amount;
    currentDamage = baseDamage * (1.0f + critDamageBonus);
}

void WeaponStats::upgradeSpeed(float amount) {
    attackSpeed += amount;
    if (attackSpeed < 0.1f) attackSpeed = 0.1f;
}

void WeaponStats::upgradeRange(float amount) {
    range += amount;
    if (range < 1.0f) range = 1.0f;
}

void WeaponStats::addElementalDamage(ElementType type, float damage) {
    switch (type) {
        case ElementType::Fire: fireDamage += damage; break;
        case ElementType::Ice: iceDamage += damage; break;
        case ElementType::Lightning: lightningDamage += damage; break;
        case ElementType::Poison: poisonDamage += damage; break;
        default: break;
    }
}

float WeaponStats::getTotalDamage() const {
    return currentDamage + fireDamage + iceDamage + lightningDamage + poisonDamage;
}

// ========== WEAPON UPGRADE SYSTEM ==========

WeaponUpgradeSystem::WeaponUpgradeSystem() {
    // Initialize upgrade paths
    upgradePaths[WeaponType::Sword] = {
        {"Sharpness", UpgradeType::Damage, 2.0f, 100},
        {"Weight Reduction", UpgradeType::Speed, 0.1f, 150},
        {"Reach", UpgradeType::Range, 0.5f, 200},
        {"Critical Edge", UpgradeType::CritChance, 0.05f, 300}
    };
    
    upgradePaths[WeaponType::Bow] = {
        {"Arrow Weight", UpgradeType::Damage, 3.0f, 120},
        {"Draw Speed", UpgradeType::Speed, 0.15f, 180},
        {"Range Extension", UpgradeType::Range, 2.0f, 250},
        {"Quick Shot", UpgradeType::AttackSpeed, 0.2f, 350}
    };
    
    upgradePaths[WeaponType::Staff] = {
        {"Spell Power", UpgradeType::Damage, 4.0f, 140},
        {"Mana Flow", UpgradeType::Speed, 0.12f, 190},
        {"Elemental Mastery", UpgradeType::ElementalDamage, 5.0f, 280},
        {"Arcane Reach", UpgradeType::Range, 1.5f, 320}
    };
}

bool WeaponUpgradeSystem::upgradeWeapon(Entity weapon, UpgradeType type, EntityManager& em) {
    auto stats = em.getComponent<WeaponStatsComponent>(weapon);
    if (!stats) return false;
    
    // Check if upgrade is available for this weapon level
    int currentLevel = stats->level;
    if (currentLevel >= 10) return false; // Max level
    
    // Get upgrade cost
    int cost = getUpgradeCost(type, currentLevel);
    
    // Check materials
    if (!hasRequiredMaterials(weapon, type, currentLevel + 1, em)) {
        return false;
    }
    
    // Apply upgrade
    applyUpgrade(stats, type, 1.0f);
    stats->level++;
    
    // Consume materials
    consumeMaterials(weapon, type, currentLevel + 1, em);
    
    // Update back in entity manager
    em.addComponent<WeaponStatsComponent>(weapon, stats);
    
    return true;
}

int WeaponUpgradeSystem::getUpgradeCost(UpgradeType type, int currentLevel) const {
    int baseCost = 100 * (currentLevel + 1);
    switch (type) {
        case UpgradeType::Damage: return baseCost * 1.2f;
        case UpgradeType::Speed: return baseCost * 1.0f;
        case UpgradeType::Range: return baseCost * 1.1f;
        case UpgradeType::CritChance: return baseCost * 1.5f;
        case UpgradeType::ElementalDamage: return baseCost * 1.3f;
        default: return baseCost;
    }
}

bool WeaponUpgradeSystem::hasRequiredMaterials(Entity weapon, UpgradeType type, int targetLevel, EntityManager& em) const {
    auto inventory = em.getComponent<InventoryComponent>(weapon);
    if (!inventory) return false;
    
    // Check each material type
    for (const auto& material : inventory->materials) {
        if (material.second.quantity >= material.second.getRequiredQuantity(targetLevel)) {
            return true;
        }
    }
    return false;
}

void WeaponUpgradeSystem::consumeMaterials(Entity weapon, UpgradeType type, int targetLevel, EntityManager& em) {
    auto inventory = em.getComponent<InventoryComponent>(weapon);
    if (!inventory) return;
    
    for (auto& material : inventory->materials) {
        int required = material.second.getRequiredQuantity(targetLevel);
        material.second.quantity -= required;
        if (material.second.quantity <= 0) {
            inventory->materials.erase(material.first);
        }
    }
    
    em.addComponent<InventoryComponent>(weapon, inventory);
}

void WeaponUpgradeSystem::applyUpgrade(std::shared_ptr<WeaponStatsComponent> stats, UpgradeType type, float multiplier) {
    switch (type) {
        case UpgradeType::Damage:
            stats->upgradeDamage(2.0f * multiplier);
            break;
        case UpgradeType::Speed:
            stats->upgradeSpeed(0.5f * multiplier);
            break;
        case UpgradeType::Range:
            stats->upgradeRange(1.0f * multiplier);
            break;
        case UpgradeType::CritChance:
            stats->critChance += 0.05f * multiplier;
            if (stats->critChance > 0.5f) stats->critChance = 0.5f;
            break;
        case UpgradeType::CritDamage:
            stats->critDamageBonus += 0.1f * multiplier;
            break;
        case UpgradeType::ElementalDamage:
            stats->addElementalDamage(ElementType::Fire, 3.0f * multiplier);
            break;
    }
}

// ========== WEAPON CRAFTING ==========

Entity WeaponUpgradeSystem::craftWeapon(WeaponType type, int tier, EntityManager& em) {
    Entity weapon = em.createEntity();
    
    auto stats = std::make_shared<WeaponStatsComponent>();
    stats->type = type;
    stats->level = 1;
    stats->tier = tier;
    
    // Base stats by type and tier
    float tierMultiplier = 1.0f + (tier - 1) * 0.5f;
    
    switch (type) {
        case WeaponType::Sword:
            stats->baseDamage = 10.0f * tierMultiplier;
            stats->attackSpeed = 1.0f;
            stats->range = 2.0f;
            break;
        case WeaponType::Bow:
            stats->baseDamage = 8.0f * tierMultiplier;
            stats->attackSpeed = 0.8f;
            stats->range = 15.0f;
            break;
        case WeaponType::Staff:
            stats->baseDamage = 12.0f * tierMultiplier;
            stats->attackSpeed = 0.6f;
            stats->range = 10.0f;
            stats->addElementalDamage(ElementType::Fire, 5.0f * tierMultiplier);
            break;
        case WeaponType::Dagger:
            stats->baseDamage = 6.0f * tierMultiplier;
            stats->attackSpeed = 1.5f;
            stats->range = 1.5f;
            stats->critChance = 0.15f;
            break;
        case WeaponType::Axe:
            stats->baseDamage = 15.0f * tierMultiplier;
            stats->attackSpeed = 0.7f;
            stats->range = 2.5f;
            break;
    }
    
    stats->currentDamage = stats->getTotalDamage();
    stats->durability = 100 * tier;
    stats->maxDurability = stats->durability;
    
    em.addComponent<WeaponStatsComponent>(weapon, stats);
    
    return weapon;
}

// ========== WEAPON DURABILITY ==========

void WeaponUpgradeSystem::updateDurability(EntityManager& em, float deltaTime) {
    auto weapons = em.getAllComponents<WeaponStatsComponent>();
    
    for (auto& pair : weapons) {
        Entity weapon = pair.first;
        auto stats = pair.second;
        
        // Durability decreases with use (would be called on attack)
        // This is just a passive check
        
        if (stats->durability <= 0) {
            // Weapon broken
            stats->currentDamage *= 0.5f; // Reduced damage when broken
        }
    }
}

bool WeaponUpgradeSystem::repairWeapon(Entity weapon, EntityManager& em) {
    auto stats = em.getComponent<WeaponStatsComponent>(weapon);
    if (!stats) return false;
    
    if (stats->durability >= stats->maxDurability) return false; // Already full
    
    // Repair costs materials
    auto inventory = em.getComponent<InventoryComponent>(weapon);
    if (!inventory) return false;
    
    int repairCost = static_cast<int>((stats->maxDurability - stats->durability) * 2);
    
    // Check for repair materials
    if (inventory->materials.find("repair_kit") != inventory->materials.end()) {
        auto& kit = inventory->materials["repair_kit"];
        if (kit.quantity >= repairCost) {
            kit.quantity -= repairCost;
            stats->durability = stats->maxDurability;
            stats->currentDamage = stats->getTotalDamage(); // Restore full damage
            em.addComponent<WeaponStatsComponent>(weapon, stats);
            return true;
        }
    }
    
    return false;
}

// ========== WEAPON ENCHANTMENT ==========

bool WeaponUpgradeSystem::addEnchantment(Entity weapon, EnchantmentType type, EntityManager& em) {
    auto stats = em.getComponent<WeaponStatsComponent>(weapon);
    if (!stats) return false;
    
    if (stats->enchantments.size() >= 3) return false; // Max 3 enchantments
    
    EnchantmentInfo info;
    info.type = type;
    info.level = 1;
    info.active = true;
    
    switch (type) {
        case EnchantmentType::Fire:
            info.name = "Fire Enchant";
            info.description = "Adds fire damage";
            stats->addElementalDamage(ElementType::Fire, 5.0f);
            break;
        case EnchantmentType::Ice:
            info.name = "Ice Enchant";
            info.description = "Slows enemies";
            stats->addElementalDamage(ElementType::Ice, 4.0f);
            break;
        case EnchantmentType::Lightning:
            info.name = "Lightning Enchant";
            info.description = "Chain lightning";
            stats->addElementalDamage(ElementType::Lightning, 6.0f);
            break;
        case EnchantmentType::Poison:
            info.name = "Poison Enchant";
            info.description = "Damage over time";
            stats->addElementalDamage(ElementType::Poison, 3.0f);
            break;
    }
    
    stats->enchantments.push_back(info);
    stats->currentDamage = stats->getTotalDamage();
    em.addComponent<WeaponStatsComponent>(weapon, stats);
    
    return true;
}

// ========== STAT QUERIES ==========

float WeaponUpgradeSystem::getWeaponDPS(Entity weapon, EntityManager& em) const {
    auto stats = em.getComponent<WeaponStatsComponent>(weapon);
    if (!stats) return 0.0f;
    
    return stats->getTotalDamage() * stats->attackSpeed;
}

int WeaponUpgradeSystem::getWeaponLevel(Entity weapon, EntityManager& em) const {
    auto stats = em.getComponent<WeaponStatsComponent>(weapon);
    return stats ? stats->level : 0;
}

float WeaponUpgradeSystem::getWeaponDurabilityPercent(Entity weapon, EntityManager& em) const {
    auto stats = em.getComponent<WeaponStatsComponent>(weapon);
    if (!stats || stats->maxDurability <= 0) return 0.0f;
    return (stats->durability / stats->maxDurability) * 100.0f;
}

} // namespace Doomnite
