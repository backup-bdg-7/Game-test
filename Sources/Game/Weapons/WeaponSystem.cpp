// WeaponSystem.cpp - C++ Weapon System Implementation

#include "WeaponSystem.hpp"
#include <algorithm>

namespace Doomnite {

// MARK: - WeaponComponent Implementation

void WeaponComponent::recalculateStats() {
    currentStats = baseStats;
    
    for (const auto& rune : socketedRunes) {
        currentStats.baseDamage *= rune->damageMultiplier;
        currentStats.fireRate *= (1.0f + rune->cooldownReduction);
    }
}

bool WeaponComponent::socketRune(std::shared_ptr<Rune> rune) {
    if (socketedRunes.size() >= maxRunes) return false;
    
    // Check for duplicate runes (optional)
    for (const auto& existing : socketedRunes) {
        if (existing->id == rune->id) return false; // Duplicate
    }
    
    socketedRunes.push_back(rune);
    recalculateStats();
    return true;
}

bool WeaponComponent::unsocketRune(int index) {
    if (index < 0 || index >= static_cast<int>(socketedRunes.size())) return false;
    
    socketedRunes.erase(socketedRunes.begin() + index);
    recalculateStats();
    return true;
}

// MARK: - WeaponAbility Implementation

bool WeaponAbility::canActivate() const {
    return currentCooldown <= 0.0f;
}

void WeaponAbility::activate() {
    currentCooldown = cooldown;
}

void WeaponAbility::update(float deltaTime) {
    if (currentCooldown > 0.0f) {
        currentCooldown -= deltaTime;
    }
}

// MARK: - ChargeShotAbility Implementation

float ChargeShotAbility::getDamageMultiplier() const {
    if (!isCharging) return 1.0f;
    return 1.0f + (chargeDamageMultiplier - 1.0f) * (currentCharge / chargeTime);
}

// MARK: - WeaponSystem Implementation

void WeaponSystem::update(EntityManager& entities, float deltaTime) {
    auto weapons = entities.componentStorage.getAllComponents<WeaponComponent>();
    
    for (auto& pair : weapons) {
        Entity entity = pair.first;
        auto weapon = pair.second;
        
        // Update weapon cooldown (fire rate)
        // (Would track lastFireTime and apply cooldown)
        
        // Update reload if reloading
        // (Would check isReloading and apply reloadTime)
    }
}

bool WeaponSystem::fireWeapon(const Entity& weaponEntity, EntityManager& entities) {
    auto weapon = entities.componentStorage.getComponent<WeaponComponent>(weaponEntity);
    if (!weapon || !weapon->canFire()) return false;
    
    // Create projectile or perform hitscan based on weapon type
    switch (weapon->type) {
        case WeaponType::Pistol:
        case WeaponType::Rifle:
        case WeaponType::SMG:
            // Hitscan weapons
            // performHitscan();
            break;
            
        case WeaponType::RocketLauncher:
        case WeaponType::Bow:
            // Projectile weapons
            // createProjectile();
            break;
            
        case WeaponType::Staff:
            // Magic projectile
            // createMagicProjectile();
            break;
            
        default:
            break;
    }
    
    weapon->currentAmmo--;
    return true;
}

// MARK: - WeaponFactory Implementation

std::shared_ptr<WeaponComponent> WeaponFactory::createStaff() {
    WeaponStats stats(15.0f, 3.0f, 80.0f, 100, 0.0f, 0.8f, 1.2f);
    auto weapon = std::make_shared<WeaponComponent>(WeaponType::Staff, stats);
    weapon->maxRunes = 4;  // Staves can hold more runes
    return weapon;
}

std::shared_ptr<WeaponComponent> WeaponFactory::createPistol() {
    WeaponStats stats(10.0f, 5.0f, 50.0f, 12, 1.5f, 0.95f, 0.8f);
    return std::make_shared<WeaponComponent>(WeaponType::Pistol, stats);
}

std::shared_ptr<WeaponComponent> WeaponFactory::createRocketLauncher() {
    WeaponStats stats(80.0f, 0.5f, 200.0f, 4, 4.0f, 0.7f, 2.0f);
    auto weapon = std::make_shared<WeaponComponent>(WeaponType::RocketLauncher, stats);
    
    // Add AOE ability
    auto aoeAbility = std::make_shared<AreaOfEffectAbility>();
    // weapon->abilities.push_back(aoeAbility); // If we had an abilities vector
    
    return weapon;
}

} // namespace Doomnite
