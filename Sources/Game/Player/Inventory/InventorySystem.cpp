// InventorySystem.cpp - FULL Production Implementation
// Complete inventory management with sorting, filtering, storage

#include "InventorySystem.hpp"
#include <algorithm>"
#include <cmath>"
#include <ctime>"

namespace Doomnite {

// ========== ITEM MANAGER ==========

Entity ItemManager::createItem(ItemType type, ItemRarity rarity, int level) {
    Entity item = entityManager.createEntity();
    
    auto itemComp = std::make_shared<ItemComponent>();
    itemComp->itemId = "item_" + std::to_string(rand() % 10000);
    itemComp->type = type;
    itemComp->rarity = rarity;
    itemComp->level = level;
    itemComp->quantity = 1;
    
    // Generate stats based on type and rarity
    generateItemStats(itemComp);
    
    // Create transform for world items
    auto transform = std::make_shared<TransformComponent>(Vec3(0, 0, 0));
    transform->scale = Vec3(1.0f, 1.0f, 1.0f);
    
    entityManager.getComponentStorage().addComponent<ItemComponent>(item, itemComp);
    entityManager.getComponentStorage().addComponent<TransformComponent>(item, transform);
    
    return item;
}

void ItemManager::generateItemStats(std::shared_ptr<ItemComponent> item) {
    float rarityMultiplier = getRarityMultiplier(item->rarity);
    float levelMultiplier = 1.0f + (item->level - 1) * 0.2f;
    
    switch (item->type) {
        case ItemType::Weapon:
            item->stats.attackDamage = 10.0f * rarityMultiplier * levelMultiplier;
            item->stats.attackSpeed = 1.0f;
            item->stats.criticalChance = 0.05f * rarityMultiplier;
            item->stats.durability = 100.0f;
            item->name = getWeaponName(item->rarity);
            item->description = "A powerful weapon";
            item->goldValue = static_cast<int>(20 * rarityMultiplier * levelMultiplier);
            break;
            
        case ItemType::Armor:
            item->stats.defense = 5.0f * rarityMultiplier * levelMultiplier;
            item->stats.healthBonus = 10.0f * rarityMultiplier;
            item->stats.durability = 150.0f;
            item->name = getArmorName(item->rarity);
            item->description = "Protective armor";
            item->goldValue = static_cast<int>(15 * rarityMultiplier * levelMultiplier);
            break;
            
        case ItemType::Potion:
            item->stats.healAmount = 50.0f * rarityMultiplier;
            item->stats.manaRestore = 30.0f * rarityMultiplier;
            item->name = getPotionName(item->rarity);
            item->description = "Restores health and mana";
            item->goldValue = static_cast<int>(5 * rarityMultiplier);
            item->quantity = 1 + (rand() % 3);
            break;
            
        case ItemType::Material:
            item->name = getMaterialName(item->rarity);
            item->description = "Crafting material";
            item->goldValue = static_cast<int>(3 * rarityMultiplier);
            item->quantity = 1 + (rand() % 5);
            break;
            
        case ItemType::QuestItem:
            item->name = "Quest Item";
            item->description = "A special quest item";
            item->goldValue = 0; // Quest items not sellable
            item->isQuestItem = true;
            break;
    }
    
    item->weight = calculateWeight(item->type, item->rarity);
}

float ItemManager::getRarityMultiplier(ItemRarity rarity) const {
    switch (rarity) {
        case ItemRarity::Common: return 1.0f;
        case ItemRarity::Uncommon: return 1.5f;
        case ItemRarity::Rare: return 2.5f;
        case ItemRarity::Epic: return 4.0f;
        case ItemRarity::Legendary: return 7.0f;
        default: return 1.0f;
    }
}

std::string ItemManager::getWeaponName(ItemRarity rarity) const {
    switch (rarity) {
        case ItemRarity::Common: return "Iron Sword";
        case ItemRarity::Uncommon: return "Steel Sword";
        case ItemRarity::Rare: return "Enchanted Blade";
        case ItemRarity::Epic: return "Dragonfang";
        case ItemRarity::Legendary: return "Godslayer";
        default: return "Sword";
    }
}

std::string ItemManager::getArmorName(ItemRarity rarity) const {
    switch (rarity) {
        case ItemRarity::Common: return "Leather Armor";
        case ItemRarity::Uncommon: return "Chain Mail";
        case ItemRarity::Rare: return "Plate Armor";
        case ItemRarity::Epic: return "Dragonscale Armor";
        case ItemRarity::Legendary: return "Titan's Guard";
        default: return "Armor";
    }
}

// ========== INVENTORY MANAGER ==========

bool InventoryManager::addItem(Entity entity, Entity item, EntityManager& em) {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
    
    if (!inventory || !itemComp) return false;
    
    // Check capacity
    if (inventory->items.size() >= inventory->capacity) {
        return false; // Inventory full
    }
    
    // Check if stackable
    if (itemComp->isStackable) {
        for (auto& invItem : inventory->items) {
            auto otherItem = em.getComponentStorage().getComponent<ItemComponent>(invItem);
            if (otherItem && otherItem->itemId == itemComp->itemId) {
                otherItem->quantity += itemComp->quantity;
                // Remove the duplicate entity
                em.destroyEntity(item);
                return true;
            }
        }
    }
    
    // Add new item
    inventory->items.push_back(item);
    inventory->totalWeight += itemComp->weight * itemComp->quantity;
    
    // Sort inventory
    sortInventory(entity, em);
    
    // Update back in entity manager
    em.getComponentStorage().addComponent<InventoryComponent>(entity, inventory);
    
    return true;
}

bool InventoryManager::removeItem(Entity entity, Entity item, EntityManager& em) {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    if (!inventory) return false;
    
    auto it = std::find(inventory->items.begin(), inventory->items.end(), item);
    if (it == inventory->items.end()) return false;
    
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(*it);
    if (itemComp) {
        inventory->totalWeight -= itemComp->weight * itemComp->quantity;
    }
    
    inventory->items.erase(it);
    
    em.getComponentStorage().addComponent<InventoryComponent>(entity, inventory);
    return true;
}

bool InventoryManager::useItem(Entity entity, Entity item, EntityManager& em) {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
    
    if (!inventory || !itemComp) return false;
    
    switch (itemComp->type) {
        case ItemType::Potion: {
            auto health = em.getComponentStorage().getComponent<HealthComponent>(entity);
            if (health) {
                float heal = itemComp->stats.healAmount;
                health->currentHealth = std::min(health->maxHealth, health->currentHealth + heal);
                em.getComponentStorage().addComponent<HealthComponent>(entity, health);
            }
            
            // Reduce quantity or remove
            itemComp->quantity--;
            if (itemComp->quantity <= 0) {
                removeItem(entity, item, em);
            } else {
                em.getComponentStorage().addComponent<ItemComponent>(item, itemComp);
            }
            return true;
        }
        
        case ItemType::Weapon:
        case ItemType::Armor: {
            // Equip the item
            EquipmentSystem equipSys;
            return equipSys.equipItem(entity, item, em);
        }
        
        default:
            return false;
    }
}

Entity InventoryManager::dropItem(Entity entity, Entity item, EntityManager& em) {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
    auto transform = em.getComponentStorage().getComponent<TransformComponent>(entity);
    
    if (!inventory || !itemComp || !transform) return Entity();
    
    // Remove from inventory
    removeItem(entity, item, em);
    
    // Place in world at entity's position
    auto itemTransform = em.getComponentStorage().getComponent<TransformComponent>(item);
    if (itemTransform) {
        itemTransform->position = transform->position + Vec3(1.0f, 0, 1.0f); // Drop slightly offset
        em.getComponentStorage().addComponent<TransformComponent>(item, itemTransform);
    }
    
    return item;
}

void InventoryManager::sortInventory(Entity entity, EntityManager& em) {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    if (!inventory) return;
    
    // Sort by type, then rarity, then level
    std::sort(inventory->items.begin(), inventory->items.end(), 
        [&em](Entity a, Entity b) {
            auto itemA = em.getComponentStorage().getComponent<ItemComponent>(a);
            auto itemB = em.getComponentStorage().getComponent<ItemComponent>(b);
            
            if (!itemA || !itemB) return false;
            
            // Sort by type first
            if (itemA->type != itemB->type) return itemA->type < itemB->type;
            
            // Then by rarity
            if (itemA->rarity != itemB->rarity) return itemA->rarity > itemB->rarity;
            
            // Then by level
            return itemA->level > itemB->level;
        });
    
    em.getComponentStorage().addComponent<InventoryComponent>(entity, inventory);
}

std::vector<Entity> InventoryManager::getItemsByType(Entity entity, ItemType type, EntityManager& em) const {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    if (!inventory) return {};
    
    std::vector<Entity> result;
    for (auto item : inventory->items) {
        auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
        if (itemComp && itemComp->type == type) {
            result.push_back(item);
        }
    }
    return result;
}

bool InventoryManager::hasItem(Entity entity, const std::string& itemId, EntityManager& em) const {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    if (!inventory) return false;
    
    for (auto item : inventory->items) {
        auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
        if (itemComp && itemComp->itemId == itemId) {
            return true;
        }
    }
    return false;
}

int InventoryManager::getItemCount(Entity entity, const std::string& itemId, EntityManager& em) const {
    auto inventory = em.getComponentStorage().getComponent<InventoryComponent>(entity);
    if (!inventory) return 0;
    
    int count = 0;
    for (auto item : inventory->items) {
        auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
        if (itemComp && itemComp->itemId == itemId) {
            count += itemComp->quantity;
        }
    }
    return count;
}

// ========== EQUIPMENT SYSTEM ==========

bool EquipmentSystem::equipItem(Entity entity, Entity item, EntityManager& em) {
    auto equipment = em.getComponentStorage().getComponent<EquipmentComponent>(entity);
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
    
    if (!equipment || !itemComp) return false;
    
    // Check level requirement
    if (itemComp->level > equipment->level) return false;
    
    // Find appropriate slot
    EquipmentSlot slot = getEquipmentSlot(itemComp->type);
    if (slot == EquipmentSlot::None) return false;
    
    // Unequip current item in that slot if any
    auto currentItem = equipment->slots[static_cast<int>(slot)];
    if (currentItem.id != 0) {
        unequipItem(entity, slot, em);
    }
    
    // Equip new item
    equipment->slots[static_cast<int>(slot)] = item;
    equipment->totalStats = equipment->totalStats + itemComp->stats;
    
    // Remove from inventory
    InventoryManager invMgr;
    invMgr.removeItem(entity, item, em);
    
    em.getComponentStorage().addComponent<EquipmentComponent>(entity, equipment);
    
    return true;
}

bool EquipmentSystem::unequipItem(Entity entity, EquipmentSlot slot, EntityManager& em) {
    auto equipment = em.getComponentStorage().getComponent<EquipmentComponent>(entity);
    if (!equipment) return false;
    
    auto item = equipment->slots[static_cast<int>(slot)];
    if (item.id == 0) return false;
    
    // Add to inventory
    InventoryManager invMgr;
    if (!invMgr.addItem(entity, item, em)) return false;
    
    // Remove from equipment
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
    if (itemComp) {
        equipment->totalStats = equipment->totalStats - itemComp->stats;
    }
    equipment->slots[static_cast<int>(slot)] = Entity();
    
    em.getComponentStorage().addComponent<EquipmentComponent>(entity, equipment);
    
    return true;
}

EquipmentSlot EquipmentSystem::getEquipmentSlot(ItemType type) const {
    switch (type) {
        case ItemType::Weapon: return EquipmentSlot::Weapon;
        case ItemType::Armor: return EquipmentSlot::Chest;
        case ItemType::Helmet: return EquipmentSlot::Helmet;
        case ItemType::Boots: return EquipmentSlot::Boots;
        case ItemType::Gloves: return EquipmentSlot::Gloves;
        case ItemType::Ring: return EquipmentSlot::Ring;
        case ItemType::Amulet: return EquipmentSlot::Amulet;
        default: return EquipmentSlot::None;
    }
}

ItemStats EquipmentSystem::getTotalStats(Entity entity, EntityManager& em) const {
    auto equipment = em.getComponentStorage().getComponent<EquipmentComponent>(entity);
    if (!equipment) return ItemStats();
    return equipment->totalStats;
}

// ========== STASH SYSTEM ==========

bool StashSystem::storeItem(Entity stash, Entity item, EntityManager& em) {
    auto stashComp = em.getComponentStorage().getComponent<StashComponent>(stash);
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
    
    if (!stashComp || !itemComp) return false;
    
    if (stashComp->items.size() >= stashComp->capacity) return false;
    
    stashComp->items.push_back(item);
    stashComp->totalValue += itemComp->goldValue * itemComp->quantity;
    
    em.getComponentStorage().addComponent<StashComponent>(stash, stashComp);
    
    return true;
}

Entity StashSystem::retrieveItem(Entity stash, int itemIndex, EntityManager& em) {
    auto stashComp = em.getComponentStorage().getComponent<StashComponent>(stash);
    if (!stashComp || itemIndex < 0 || itemIndex >= stashComp->items.size()) {
        return Entity();
    }
    
    Entity item = stashComp->items[itemIndex];
    stashComp->items.erase(stashComp->items.begin() + itemIndex);
    
    auto itemComp = em.getComponentStorage().getComponent<ItemComponent>(item);
    if (itemComp) {
        stashComp->totalValue -= itemComp->goldValue * itemComp->quantity;
    }
    
    em.getComponentStorage().addComponent<StashComponent>(stash, stashComp);
    
    return item;
}

void StashSystem::upgradeStash(Entity stash, EntityManager& em) {
    auto stashComp = em.getComponentStorage().getComponent<StashComponent>(stash);
    if (!stashComp) return;
    
    stashComp->capacity += 20;
    stashComp->level++;
    
    em.getComponentStorage().addComponent<StashComponent>(stash, stashComp);
}

} // namespace Doomnite
