// ArmorSystem.cpp - C++ Armor System Implementation

#include "ArmorSystem.hpp"
#include <algorithm>

namespace Doomnite {

// MARK: - ArmorPiece Implementation

float ArmorPiece::getTotalArmor() const {
    float total = armorValue;
    for (const auto& ench : enchantments) {
        if (ench->type == EnchantmentType::DamageResitance) {
            total += ench->magnitude;
        }
    }
    return total;
}

bool ArmorPiece::addEnchantment(std::shared_ptr<ArmorEnchantment> enchantment) {
    // Limit enchantments based on rarity
    int maxEnchants = 0;
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

void ArmorPiece::removeEnchantment(int index) {
    if (index >= 0 && index < static_cast<int>(enchantments.size())) {
        enchantments.erase(enchantments.begin() + index);
    }
}

void ArmorPiece::applyEffectsToPlayer(const Entity& player, EntityManager& entities) const {
    auto combat = entities.componentStorage.getComponent<CombatComponent>(player);
    if (!combat) return;
    
    for (const auto& ench : enchantments) {
        switch (ench->type) {
            case EnchantmentType::HealthBoost:
                combat->maxHealth += ench->magnitude;
                break;
            case EnchantmentType::DamageResitance:
                combat->damageResitance += ench->magnitude;
                break;
            case EnchantmentType::ElementalResitance:
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

// MARK: - ArmorComponent Implementation

bool ArmorComponent::equipPiece(std::shared_ptr<ArmorPiece> piece, EntityManager& entities, const Entity& player) {
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

void ArmorComponent::unequipSlot(ArmorSlot slot, EntityManager& entities, const Entity& player) {
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

void ArmorComponent::recalculate(EntityManager& entities, const Entity& player) {
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
    if (movementPenalty > 0.5f) movementPenalty = 0.5f;  // Max 50% slowdown
}

std::vector<std::shared_ptr<ArmorPiece>> ArmorComponent::getAllEquippedPieces() const {
    std::vector<std::shared_ptr<ArmorPiece>> pieces;
    
    if (helmet) pieces.push_back(helmet);
    if (chest) pieces.push_back(chest);
    if (gloves) pieces.push_back(gloves);
    if (boots) pieces.push_back(boots);
    if (shield) pieces.push_back(shield);
    pieces.insert(pieces.end(), accessories.begin(), accessories.end());
    
    return pieces;
}

float ArmorComponent::getDamageResistance() const {
    // Convert armor to damage resistance (simplified formula)
    return std::min(totalArmor / (totalArmor + 100.0f), 0.8f);  // Max 80% resistance
}

// MARK: - ArmorSet Implementation

void ArmorSet::addSetBonus(int pieceCount, std::shared_ptr<ArmorEnchantment> bonus) {
    setBonuses[pieceCount].push_back(bonus);
}

std::vector<std::shared_ptr<ArmorEnchantment>> ArmorSet::getBonusesForCount(int count) {
    std::vector<std::shared_ptr<ArmorEnchantment>> bonuses;
    
    for (const auto& pair : setBonuses) {
        if (pair.first <= count) {
            bonuses.insert(bonuses.end(), pair.second.begin(), pair.second.end());
        }
    }
    
    return bonuses;
}

// MARK: - ArmorSystem Implementation

void ArmorSystem::update(EntityManager& entities, float deltaTime) {
    auto armorComponents = entities.componentStorage.getAllComponents<ArmorComponent>();
    
    for (auto& pair : armorComponents) {
        Entity entity = pair.first;
        auto armorComp = pair.second;
        
        // Update durability (if we had durability loss on hit)
        // Check for broken armor pieces
    }
}

bool ArmorSystem::equipArmorPiece(const Entity& entity, EntityManager& entities,
                                    std::shared_ptr<ArmorPiece> piece) {
    auto armorComp = entities.componentStorage.getComponent<ArmorComponent>(entity);
    if (!armorComp) {
        armorComp = std::make_shared<ArmorComponent>();
        entities.componentStorage.addComponent<ArmorComponent>(entity, armorComp);
    }
    
    return armorComp->equipPiece(piece, entities, entity);
}

} // namespace Doomnite
