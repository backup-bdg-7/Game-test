// GodRuinsSystem.hpp - C++ God Ruins System for Doomnite
// Elder Scrolls-inspired powerful artifacts with insane damage/effects

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - God Ruin Types

enum class RuinType {
    Weapon,    // Applies to weapons
    Armor,     // Applies to armor
    Player,    // Applies to player directly
    Global     // Global effect
};

enum class RuinEffectType {
    DamageMultiplier,   // Multiply all damage
    ElementDamage,      // Add elemental damage
    CooldownReduction,  // Reduce skill/weapon cooldowns
    HealthBoost,        // Increase max health
    ManaBoost,          // Increase max mana
    SpeedBoost,         // Increase movement speed
    ArmorPenetration,  // Ignore enemy armor
    LifeSteal,          // Heal based on damage dealt
    ChainEffect,        // Chain attacks to nearby enemies
    AOEEffect,          // Add area of effect
    SetBonus            // Part of a set
};

// MARK: - God Ruin Effect

struct GodRuinEffect {
    RuinEffectType type;
    float magnitude;          // Effect strength
    float duration;           // 0 = permanent while equipped
    float radius;             // For AOE effects
    ElementType element;      // For elemental effects
    
    GodRuinEffect(RuinEffectType effectType, float mag = 1.0f, float dur = 0.0f,
                   float rad = 0.0f, ElementType elem = ElementType::None)
        : type(effectType), magnitude(mag), duration(dur), radius(rad), element(elem) {}
};

// MARK: - God Ruin (Artifact)

struct GodRuin {
    std::string id;
    std::string name;
    std::string description;
    Rarity rarity;
    RuinType type;
    std::vector<GodRuinEffect> effects;
    std::string setName;      // For set bonuses
    int maxStacks;           // How many times it can stack (usually 1)
    std::string requiredDungeon;  // Where it can be found
    
    GodRuin(const std::string& ruinId, const std::string& ruinName,
            const std::string& desc, Rarity rar = Rarity::GodTier,
            RuinType ruinType = RuinType::Weapon,
            const std::vector<GodRuinEffect>& effs = {},
            const std::string& set = "", int stacks = 1,
            const std::string& dungeon = "")
        : id(ruinId), name(ruinName), description(desc), rarity(rar), type(ruinType),
          effects(effs), setName(set), maxStacks(stacks), requiredDungeon(dungeon) {}
    
    // Get total damage multiplier from all effects
    float getTotalDamageMultiplier() const {
        float total = 1.0f;
        for (const auto& effect : effects) {
            if (effect.type == RuinEffectType::DamageMultiplier) {
                total *= effect.magnitude;
            }
        }
        return total;
    }
    
    // Check if part of a set
    bool isSetItem() const {
        return !setName.empty();
    }
};

// MARK: - God Ruin Component (ECS)

struct GodRuinComponent : public Component {
    std::vector<std::shared_ptr<GodRuin>> activeRuines;
    std::unordered_map<std::string, int> setCounts;  // Set name -> count
    float totalDamageMultiplier;
    float totalCooldownReduction;
    float lifeStealPercentage;
    
    GodRuinComponent() : totalDamageMultiplier(1.0f), totalCooldownReduction(0.0f),
                          lifeStealPercentage(0.0f) {}
    
    bool addRuin(std::shared_ptr<GodRuin> ruin) {
        // Check if already have this ruin (unless it stacks)
        for (const auto& existing : activeRuines) {
            if (existing->id == ruin->id && existing->maxStacks <= 1) {
                return false; // Duplicate non-stacking ruin
            }
        }
        
        activeRuines.push_back(ruin);
        recalculateStats();
        return true;
    }
    
    bool removeRuin(const std::string& ruinId) {
        auto it = std::remove_if(activeRuines.begin(), activeRuines.end(),
                                [&ruinId](const auto& ruin) { return ruin->id == ruinId; });
        
        if (it != activeRuines.end()) {
            activeRuines.erase(it, activeRuines.end());
            recalculateStats();
            return true;
        }
        return false;
    }
    
    void recalculateStats() {
        totalDamageMultiplier = 1.0f;
        totalCooldownReduction = 0.0f;
        lifeStealPercentage = 0.0f;
        setCounts.clear();
        
        for (const auto& ruin : activeRuines) {
            for (const auto& effect : ruin->effects) {
                switch (effect.type) {
                    case RuinEffectType::DamageMultiplier:
                        totalDamageMultiplier *= effect.magnitude;
                        break;
                    case RuinEffectType::CooldownReduction:
                        totalCooldownReduction += effect.magnitude;
                        break;
                    case RuinEffectType::LifeSteal:
                        lifeStealPercentage += effect.magnitude;
                        break;
                    default:
                        break;
                }
            }
            
            // Track set items
            if (!ruin->setName.empty()) {
                setCounts[ruin->setName]++;
            }
        }
        
        // Apply set bonuses
        applySetBonuses();
    }
    
    void applySetBonuses() {
        for (const auto& pair : setCounts) {
            const std::string& setName = pair.first;
            int count = pair.second;
            
            // Example set bonus: "Ruin of the Ancients" set
            if (setName == "ancient_power" && count >= 3) {
                totalDamageMultiplier *= 1.5f;  // 3-piece bonus: +50% damage
            }
            if (setName == "ancient_power" && count >= 5) {
                totalDamageMultiplier *= 1.3f;  // 5-piece bonus: another +30%
                lifeStealPercentage += 0.10f;    // +10% life steal
            }
        }
    }
    
    // Get effect magnitude for a specific type
    float getEffectMagnitude(RuinEffectType type) const {
        float total = 0.0f;
        for (const auto& ruin : activeRuines) {
            for (const auto& effect : ruin->effects) {
                if (effect.type == type) {
                    total += effect.magnitude;
                }
            }
        }
        return total;
    }
};

// MARK: - Predefined God Ruins (Elder Scrolls-inspired)

namespace GodRuines {
    
    // Fire Ruins
    static GodRuin RuinOfInferno("ruin_inferno", "Ruin of Inferno",
                                    "Ancient fire ruin that engulfs enemies in flames",
                                    Rarity::GodTier, RuinType::Weapon, {
                                        GodRuinEffect(RuinEffectType::DamageMultiplier, 2.0f),
                                        GodRuinEffect(RuinEffectType::ElementDamage, 50.0f, 0.0f, 0.0f, ElementType::Fire),
                                        GodRuinEffect(RuinEffectType::AOEEffect, 5.0f, 0.0f, 3.0f)
                                    }, "ancient_power", 1, "Fire Temple");
    
    static GodRuin RuinOfTempest("ruin_tempest", "Ruin of Tempest",
                                   "Lightning ruin that chains destruction to nearby foes",
                                   Rarity::GodTier, RuinType::Weapon, {
                                       GodRuinEffect(RuinEffectType::DamageMultiplier, 2.5f),
                                       GodRuinEffect(RuinEffectType::ElementDamage, 40.0f, 0.0f, 0.0f, ElementType::Lightning),
                                       GodRuinEffect(RuinEffectType::ChainEffect, 3.0f)  // Chains to 3 enemies
                                   }, "ancient_power", 1, "Storm Peak");
    
    static GodRuin RuinOfFrost("ruin_frost", "Ruin of Frost",
                                 "Ice ruin that freezes enemies in their tracks",
                                 Rarity::GodTier, RuinType::Weapon, {
                                     GodRuinEffect(RuinEffectType::DamageMultiplier, 2.0f),
                                     GodRuinEffect(RuinEffectType::ElementDamage, 45.0f, 0.0f, 0.0f, ElementType::Ice),
                                     GodRuinEffect(RuinEffectType::CooldownReduction, 0.3f)  // 30% CDR
                                 }, "ancient_power", 1, "Frozen Citadel");
    
    // Multi-stat Ruins
    static GodRuin RuinOfTitans("ruin_titans", "Ruin of Titans",
                                   "Ancient ruin of pure power and durability",
                                   Rarity::GodTier, RuinType::Player, {
                                       GodRuinEffect(RuinEffectType::DamageMultiplier, 3.0f),
                                       GodRuinEffect(RuinEffectType::HealthBoost, 500.0f),
                                       GodRuinEffect(RuinEffectType::ArmorPenetration, 0.5f)  // 50% armor pen
                                   }, "", 1, "Titan's Rest");
    
    static GodRuin RuinOfVampires("ruin_vampire", "Ruin of Vampires",
                                     "Feeds on the life force of your enemies",
                                     Rarity::GodTier, RuinType::Weapon, {
                                         GodRuinEffect(RuinEffectType::DamageMultiplier, 2.0f),
                                         GodRuinEffect(RuinEffectType::LifeSteal, 0.15f)  // 15% life steal
                                     }, "", 1, "Vampire's Lair");
    
    // Set items
    static GodRuin AncientPowerHelm("ancient_helm", "Ancient Power Helm",
                                      "Part of the Ancient Power set",
                                      Rarity::Legendary, RuinType::Armor, {},
                                      "ancient_power", 1, "Ancient Vault");
    
    static GodRuin AncientPowerChest("ancient_chest", "Ancient Power Chest",
                                       "Part of the Ancient Power set",
                                       Rarity::Legendary, RuinType::Armor, {},
                                       "ancient_power", 1, "Ancient Vault");
    
    static std::vector<std::shared_ptr<GodRuin>> AllGodRuines = {
        std::make_shared<GodRuin>(RuinOfInferno),
        std::make_shared<GodRuin>(RuinOfTempest),
        std::make_shared<GodRuin>(RuinOfFrost),
        std::make_shared<GodRuin>(RuinOfTitans),
        std::make_shared<GodRuin>(RuinOfVampires),
        std::make_shared<GodRuin>(AncientPowerHelm),
        std::make_shared<GodRuin>(AncientPowerChest)
    };
}

// MARK: - God Ruins System

class GodRuinsSystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        auto ruinComponents = entities.componentStorage.getAllComponents<GodRuinComponent>();
        
        for (auto& pair : ruinComponents) {
            Entity entity = pair.first;
            auto ruinComp = pair.second;
            
            // Update any timed effects (if we had duration-based ruin effects)
            // For now, god ruins are permanent while equipped
        }
    }
    
    // Helper to find ruin by ID
    static std::shared_ptr<GodRuin> findRuinById(const std::string& id) {
        for (const auto& ruin : GodRuines::AllGodRuines) {
            if (ruin->id == id) {
                return ruin;
            }
        }
        return nullptr;
    }
    
    // Apply ruin effects to damage calculation
    static float applyRuinDamageMultiplier(const Entity& entity, EntityManager& entities, float baseDamage) {
        auto ruinComp = entities.componentStorage.getComponent<GodRuinComponent>(entity);
        if (!ruinComp) return baseDamage;
        
        return baseDamage * ruinComp->totalDamageMultiplier;
    }
};

} // namespace Doomnite
