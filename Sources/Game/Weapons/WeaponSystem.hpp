// WeaponSystem.hpp - C++ Weapon System for Doomnite
// Includes weapon types, rune sockets, and upgrade mechanics

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Weapon Types

enum class WeaponType {
    Pistol,
    Rifle,
    Shotgun,
    SMG,
    Sniper,
    RocketLauncher,
    Staff,
    Sword,
    Dagger,
    Bow
};

// MARK: - Element Type

enum class ElementType {
    None,
    Fire,
    Ice,
    Lightning,
    Poison,
    Arcane,
    Holy,
    Dark
};

// MARK: - Rune

struct Rune {
    std::string id;
    std::string name;
    ElementType element;
    float damageMultiplier;  // 1.0 = no change
    float elementDamage;      // Additional elemental damage
    float cooldownReduction;  // 0.0 to 1.0
    std::string description;
    
    Rune(const std::string& runeId, const std::string& runeName, ElementType elem = ElementType::None,
         float dmgMult = 1.0f, float elemDmg = 0.0f, float cdRed = 0.0f,
         const std::string& desc = "")
        : id(runeId), name(runeName), element(elem), damageMultiplier(dmgMult),
          elementDamage(elemDmg), cooldownReduction(cdRed), description(desc) {}
};

// MARK: - Weapon Stat Base

struct WeaponStats {
    float baseDamage;
    float fireRate;      // Shots per second
    float range;
    int maxAmmo;
    float reloadTime;
    float accuracy;       // 0.0 to 1.0
    float recoil;
    
    WeaponStats(float dmg = 10.0f, float rate = 5.0f, float rng = 100.0f,
                int ammo = 30, float reload = 2.0f, float acc = 0.9f, float rec = 1.0f)
        : baseDamage(dmg), fireRate(rate), range(rng), maxAmmo(ammo),
          reloadTime(reload), accuracy(acc), recoil(rec) {}
};

// MARK: - Weapon Component (ECS)

struct WeaponComponent : public Component {
    WeaponType type;
    WeaponStats baseStats;
    WeaponStats currentStats;  // Modified by runes
    int currentAmmo;
    bool isReloading;
    float lastFireTime;
    std::vector<std::shared_ptr<Rune>> socketedRunes;
    int maxRunes;
    std::vector<std::string> unlockedAbilityIds;
    
    WeaponComponent(WeaponType weaponType = WeaponType::Pistol, const WeaponStats& stats = WeaponStats())
        : type(weaponType), baseStats(stats), currentStats(stats), currentAmmo(stats.maxAmmo),
          isReloading(false), lastFireTime(0.0f), maxRunes(3) {}
    
    bool canFire() const {
        return !isReloading && currentAmmo > 0;
    }
    
    float getFireInterval() const {
        return 1.0f / currentStats.fireRate;
    }
    
    // Apply rune modifications
    void recalculateStats() {
        currentStats = baseStats;
        
        for (const auto& rune : socketedRunes) {
            currentStats.baseDamage *= rune->damageMultiplier;
            currentStats.fireRate *= (1.0f + rune->cooldownReduction);  // Reduced cooldown = higher fire rate
            // Add more rune effects as needed
        }
    }
    
    bool socketRune(std::shared_ptr<Rune> rune) {
        if (socketedRunes.size() >= maxRunes) return false;
        socketedRunes.push_back(rune);
        recalculateStats();
        return true;
    }
    
    bool unsocketRune(int index) {
        if (index < 0 || index >= socketedRunes.size()) return false;
        socketedRunes.erase(socketedRunes.begin() + index);
        recalculateStats();
        return true;
    }
};

// MARK: - Weapon Abilities (Destiny-inspired exotic weapons)

struct WeaponAbility {
    std::string id;
    std::string name;
    std::string description;
    float cooldown;
    float currentCooldown;
    bool isPassive;
    
    WeaponAbility(const std::string& abilityId, const std::string& abilityName,
                  const std::string& desc = "", float cd = 10.0f, bool passive = false)
        : id(abilityId), name(abilityName), description(desc), cooldown(cd),
          currentCooldown(0.0f), isPassive(passive) {}
    
    bool canActivate() const {
        return currentCooldown <= 0.0f;
    }
    
    void activate() {
        currentCooldown = cooldown;
    }
    
    void update(float deltaTime) {
        if (currentCooldown > 0.0f) {
            currentCooldown -= deltaTime;
        }
    }
};

// MARK: - Specific Weapon Abilities

struct ChargeShotAbility : public WeaponAbility {
    float chargeTime;
    float chargeDamageMultiplier;
    bool isCharging;
    float currentCharge;
    
    ChargeShotAbility() : WeaponAbility("charge_shot", "Charge Shot", "Hold to charge for increased damage", 0.5f, false),
                         chargeTime(2.0f), chargeDamageMultiplier(3.0f), isCharging(false), currentCharge(0.0f) {}
    
    float getDamageMultiplier() const {
        if (!isCharging) return 1.0f;
        return 1.0f + (chargeDamageMultiplier - 1.0f) * (currentCharge / chargeTime);
    }
};

struct RicochetAbility : public WeaponAbility {
    int maxRicochets;
    float ricochetRange;
    
    RicochetAbility() : WeaponAbility("ricochet", "Ricochet", "Projectiles bounce off surfaces", 0.0f, true),
                       maxRicochets(3), ricochetRange(50.0f) {}
};

struct AreaOfEffectAbility : public WeaponAbility {
    float aoeRadius;
    float aoeDamageMultiplier;
    
    AreaOfEffectAbility() : WeaponAbility("aoe_attack", "Area Blast", "Explosive shots dealing AOE damage", 5.0f, false),
                          aoeRadius(5.0f), aoeDamageMultiplier(0.5f) {}
};

// MARK: - Weapon System

class WeaponSystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        // Update weapon cooldowns
        auto weapons = entities.componentStorage.getAllComponents<WeaponComponent>();
        
        for (auto& pair : weapons) {
            Entity entity = pair.first;
            auto weapon = pair.second;
            
            // Update abilities
            for (const auto& abilityId : weapon->unlockedAbilityIds) {
                // In real implementation, would get ability and update cooldown
            }
            
            // Update reload
            // (Would check if reloading and apply reload time)
        }
    }
    
    bool fireWeapon(const Entity& weaponEntity, EntityManager& entities) {
        auto weapon = entities.componentStorage.getComponent<WeaponComponent>(weaponEntity);
        if (!weapon || !weapon->canFire()) return false;
        
        // Create projectile or perform hitscan
        // (Implementation would depend on weapon type)
        
        weapon->currentAmmo--;
        return true;
    }
};

// MARK: - Predefined Runes (Minecraft-style)

namespace Runes {
    static Rune FireRune("rune_fire", "Rune of Fire", ElementType::Fire, 1.0f, 10.0f, 0.0f,
                          "Adds fire damage to weapon attacks");
    
    static Rune IceRune("rune_ice", "Rune of Ice", ElementType::Ice, 1.0f, 8.0f, 0.0f,
                         "Adds frost damage and slows enemies");
    
    static Rune LightningRune("rune_lightning", "Rune of Lightning", ElementType::Lightning, 1.0f, 12.0f, 0.0f,
                              "Adds lightning damage with chain effect");
    
    static Rune PoisonRune("rune_poison", "Rune of Poison", ElementType::Poison, 1.0f, 5.0f, 0.0f,
                            "Adds poison damage over time");
    
    static Rune PowerRune("rune_power", "Rune of Power", ElementType::None, 1.5f, 0.0f, 0.0f,
                           "Increases weapon damage by 50%");
    
    static Rune HasteRune("rune_haste", "Rune of Haste", ElementType::None, 1.0f, 0.0f, 0.3f,
                           "Reduces weapon cooldown by 30%");
    
    static std::vector<Rune> AllRunes = {FireRune, IceRune, LightningRune, PoisonRune, PowerRune, HasteRune};
}

// MARK: - Weapon Factory

class WeaponFactory {
public:
    static std::shared_ptr<WeaponComponent> createStaff() {
        WeaponStats stats(15.0f, 3.0f, 80.0f, 100, 0.0f, 0.8f, 1.2f);
        auto weapon = std::make_shared<WeaponComponent>(WeaponType::Staff, stats);
        weapon->maxRunes = 4;  // Staves can hold more runes
        return weapon;
    }
    
    static std::shared_ptr<WeaponComponent> createPistol() {
        WeaponStats stats(10.0f, 5.0f, 50.0f, 12, 1.5f, 0.95f, 0.8f);
        return std::make_shared<WeaponComponent>(WeaponType::Pistol, stats);
    }
    
    static std::shared_ptr<WeaponComponent> createRocketLauncher() {
        WeaponStats stats(80.0f, 0.5f, 200.0f, 4, 4.0f, 0.7f, 2.0f);
        auto weapon = std::make_shared<WeaponComponent>(WeaponType::RocketLauncher, stats);
        
        // Add AOE ability
        auto aoeAbility = std::make_shared<AreaOfEffectAbility>();
        // Would add to weapon's abilities
        
        return weapon;
    }
};

} // namespace Doomnite
