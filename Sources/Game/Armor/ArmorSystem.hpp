// ArmorSystem.hpp - C++ Armor System for Doomnite
// Multi-layered defensive equipment with set bonuses and enchantments

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Armor Slot

enum class ArmorSlot {
    Helmet,
    Chest,
    Gloves,
    Boots,
    Shield,
    Ring,      // Accessories
    Amulet
};

// MARK: - Armor Type (Light/Medium/Heavy)

enum class ArmorType {
    Light,    // Rogue/Archer armor - speed bonus
    Medium,   // Warrior armor - balanced
    Heavy     // Tank armor - max protection, movement penalty
};

// MARK: - Armor Enchantment

struct ArmorEnchantment {
    std::string id;
    std::string name;
    EnchantmentType type;
    float magnitude;
    ElementType element;  // For elemental resistances
    float duration;       // 0 = permanent
    
    ArmorEnchantment(const std::string& enchId, const std::string& enchName,
                     EnchantmentType enchType, float mag = 1.0f,
                     ElementType elem = ElementType::None, float dur = 0.0f)
        : id(enchId), name(enchName), type(enchType), magnitude(mag),
          element(elem), duration(dur) {}
};

enum class EnchantmentType {
    HealthBoost,
    StaminaBoost,
    ManaBoost,
    DamageResistance,
    ElementalResistance,
    MovementSpeed,
    JumpBoost,
    Luck,           // Better loot drops
    Thorns,         // Reflect damage
    LifeSteal
};

// MARK: - Armor Piece

struct ArmorPiece {
    std::string id;
    std::string name;
    ArmorSlot slot;
    ArmorType type;
    Rarity rarity;
    float armorValue;       // Base protection
    float durability;        // Current durability
    float maxDurability;
    std::vector<std::shared_ptr<ArmorEnchantment>> enchantments;
    std::string setID;      // For set bonuses
    int requiredLevel;
    float weight;            // Affects movement speed
    
    ArmorPiece(const std::string& armorId, const std::string& armorName,
                ArmorSlot armSlot, ArmorType armType, Rarity rar = Rarity::Common,
                float armor = 10.0f, float dur = 100.0f, int level = 1,
                float wgt = 1.0f, const std::string& set = "")
        : id(armorId), name(armorName), slot(armSlot), type(armType),
          rarity(rar), armorValue(armor), durability(dur), maxDurability(dur),
          setID(set), requiredLevel(level), weight(wgt) {}
    
    float getTotalArmor() const {
        float total = armorValue;
        for (const auto& ench : enchantments) {
            if (ench->type == EnchantmentType::DamageResistance) {
                total += ench->magnitude;
            }
        }
        return total;
    }
    
    bool addEnchantment(std::shared_ptr<ArmorEnchantment> enchantment) {
        // Limit enchantments based on rarity
        int maxEnchants = 1;
        switch (rarity) {
            case Rarity::Common: maxEnchants = 0; break;
            case Rarity::Uncommon: maxEnchants = 1; break;
            case Rarity::Rare: maxEnchants = 2; break;
            case Rarity::Epic: maxEnchants = 3; break;
            case Rarity::Legendary: maxEnchants = 4; break;
            case Rarity::GodTier: maxEnchants = 6; break;
        }
        
        if (static_cast<int>(enchantments.size()) >= maxEnchants) return false;
        
        enchantments.push_back(enchantment);
        return true;
    }
    
    void removeEnchantment(int index) {
        if (index >= 0 && index < static_cast<int>(enchantments.size())) {
            enchantments.erase(enchantments.begin() + index);
        }
    }
    
    // Apply enchantment effects to player stats
    void applyEffectsToPlayer(const Entity& player, EntityManager& entities) const {
        auto combat = entities.componentStorage.getComponent<CombatComponent>(player);
        if (!combat) return;
        
        for (const auto& ench : enchantments) {
            switch (ench->type) {
                case EnchantmentType::HealthBoost:
                    combat->maxHealth += ench->magnitude;
                    break;
                case EnchantmentType::DamageResistance:
                    combat->damageResistance += ench->magnitude;
                    break;
                case EnchantmentType::ElementalResistance:
                    // Would apply to specific element resistance
                    break;
                case EnchantmentType::MovementSpeed:
                    // Would modify movement speed
                    break;
                default:
                    break;
            }
        }
        
        entities.componentStorage.addComponent<CombatComponent>(player, combat);
    }
};

// MARK: - Armor Set

struct ArmorSet {
    std::string id;
    std::string name;
    std::string description;
    Rarity rarity;
    std::vector<std::string> pieceIds;  // Helmet, Chest, etc.
    std::unordered_map<int, std::vector<std::shared_ptr<ArmorEnchantment>>> setBonuses;  // Piece count -> bonuses
    
    ArmorSet(const std::string& setID, const std::string& setName,
              const std::string& desc, Rarity rar = Rarity::Rare)
        : id(setID), name(setName), description(desc), rarity(rar) {}
    
    void addSetBonus(int pieceCount, std::shared_ptr<ArmorEnchantment> bonus) {
        setBonuses[pieceCount].push_back(bonus);
    }
    
    std::vector<std::shared_ptr<ArmorEnchantment>> getBonusesForCount(int count) {
        std::vector<std::shared_ptr<ArmorEnchantment>> bonuses;
        
        for (const auto& pair : setBonuses) {
            if (pair.first <= count) {
                bonuses.insert(bonuses.end(), pair.second.begin(), pair.second.end());
            }
        }
        
        return bonuses;
    }
};

// MARK: - Armor Component (ECS)

struct ArmorComponent : public Component {
    std::shared_ptr<ArmorPiece> helmet;
    std::shared_ptr<ArmorPiece> chest;
    std::shared_ptr<ArmorPiece> gloves;
    std::shared_ptr<ArmorPiece> boots;
    std::shared_ptr<ArmorPiece> shield;
    std::vector<std::shared_ptr<ArmorPiece>> accessories;  // Rings, amulets
    
    float totalArmor;
    float movementPenalty;  // 0.0 to 1.0
    std::unordered_map<std::string, int> setCounts;  // Set ID -> count
    
    ArmorComponent() : totalArmor(0.0f), movementPenalty(0.0f) {}
    
    bool equipPiece(std::shared_ptr<ArmorPiece> piece, EntityManager& entities, const Entity& player) {
        switch (piece->slot) {
            case ArmorSlot::Helmet:
                helmet = piece;
                break;
            case ArmorSlot::Chest:
                chest = piece;
                break;
            case ArmorSlot::Gloves:
                gloves = piece;
                break;
            case ArmorSlot::Boots:
                boots = piece;
                break;
            case ArmorSlot::Shield:
                shield = piece;
                break;
            case ArmorSlot::Ring:
            case ArmorSlot::Amulet:
                accessories.push_back(piece);
                break;
        }
        
        recalculate(entities, player);
        return true;
    }
    
    void unequipSlot(ArmorSlot slot, EntityManager& entities, const Entity& player) {
        switch (slot) {
            case ArmorSlot::Helmet: helmet.reset(); break;
            case ArmorSlot::Chest: chest.reset(); break;
            case ArmorSlot::Gloves: gloves.reset(); break;
            case ArmorSlot::Boots: boots.reset(); break;
            case ArmorSlot::Shield: shield.reset(); break;
            default: break;
        }
        
        recalculate(entities, player);
    }
    
    void recalculate(EntityManager& entities, const Entity& player) {
        totalArmor = 0.0f;
        movementPenalty = 0.0f;
        setCounts.clear();
        
        // Sum up all equipped pieces
        std::vector<std::shared_ptr<ArmorPiece>> allPieces = getAllEquippedPieces();
        
        for (const auto& piece : allPieces) {
            totalArmor += piece->getTotalArmor();
            movementPenalty += piece->weight * 0.1f;  // Weight affects movement
            
            // Track set items
            if (!piece->setID.empty()) {
                setCounts[piece->setID]++;
            }
            
            // Apply individual enchantment effects
            piece->applyEffectsToPlayer(player, entities);
        }
        
        // Apply set bonuses
        for (const auto& pair : setCounts) {
            const std::string& setId = pair.first;
            int count = pair.second;
            
            // Find set and apply bonuses
            auto set = ArmorSets::getSetByID(setId);
            if (set) {
                auto bonuses = set->getBonusesForCount(count);
                for (const auto& bonus : bonuses) {
                    // Apply set bonus effects
                    // (Would need to implement bonus application)
                }
            }
        }
        
        // Clamp movement penalty
        movementPenalty = std::min(movementPenalty, 0.5f);  // Max 50% slowdown
    }
    
    std::vector<std::shared_ptr<ArmorPiece>> getAllEquippedPieces() const {
        std::vector<std::shared_ptr<ArmorPiece>> pieces;
        
        if (helmet) pieces.push_back(helmet);
        if (chest) pieces.push_back(chest);
        if (gloves) pieces.push_back(gloves);
        if (boots) pieces.push_back(boots);
        if (shield) pieces.push_back(shield);
        pieces.insert(pieces.end(), accessories.begin(), accessories.end());
        
        return pieces;
    }
    
    float getDamageResistance() const {
        // Convert armor to damage resistance (simplified formula)
        return std::min(totalArmor / (totalArmor + 100.0f), 0.8f);  // Max 80% resistance
    }
};

// MARK: - Predefined Armor Sets

namespace ArmorSets {
    
    // Light Armor Set: "Wind Walker" (Speed bonus)
    static ArmorSet WindWalkerSet("set_windwalker", "Wind Walker",
                                    "Light armor set that enhances speed and agility",
                                    Rarity::Epic);
    
    // Medium Armor Set: "Iron Will" (Balanced)
    static ArmorSet IronWillSet("set_ironwill", "Iron Will",
                                 "Medium armor set with balanced protection",
                                 Rarity::Rare);
    
    // Heavy Armor Set: "Titan's Bulwark" (Tank)
    static ArmorSet TitansBulwarkSet("set_titan_bulwark", "Titan's Bulwark",
                                       "Heavy armor set for ultimate protection",
                                       Rarity::Legendary);
    
    // God-Tier Set: "Ancient Dragon" (Ultimate)
    static ArmorSet AncientDragonSet("set_ancient_dragon", "Ancient Dragon",
                                       "God-tier armor forged from dragon scales",
                                       Rarity::GodTier);
    
    static std::vector<std::shared_ptr<ArmorSet>> AllSets = {
        std::make_shared<ArmorSet>(WindWalkerSet),
        std::make_shared<ArmorSet>(IronWillSet),
        std::make_shared<ArmorSet>(TitansBulwarkSet),
        std::make_shared<ArmorSet>(AncientDragonSet)
    };
    
    static std::shared_ptr<ArmorSet> getSetByID(const std::string& id) {
        for (const auto& set : AllSets) {
            if (set->id == id) return set;
        }
        return nullptr;
    }
}

// MARK: - Armor System

class ArmorSystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        auto armorComponents = entities.componentStorage.getAllComponents<ArmorComponent>();
        
        for (auto& pair : armorComponents) {
            Entity entity = pair.first;
            auto armorComp = pair.second;
            
            // Update durability (if we had durability loss on hit)
            // Check for broken armor pieces
        }
    }
    
    static bool equipArmorPiece(const Entity& entity, EntityManager& entities,
                                std::shared_ptr<ArmorPiece> piece) {
        auto armorComp = entities.componentStorage.getComponent<ArmorComponent>(entity);
        if (!armorComp) {
            armorComp = std::make_shared<ArmorComponent>();
            entities.componentStorage.addComponent<ArmorComponent>(entity, armorComp);
        }
        
        return armorComp->equipPiece(piece, entities, entity);
    }
};

} // namespace Doomnite
