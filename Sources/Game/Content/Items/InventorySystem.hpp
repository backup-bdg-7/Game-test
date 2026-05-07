// InventorySystem.hpp - C++ Inventory System for Doomnite
// Handles items, equipment, storage, and item usage.

#pragma once#

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Item Rarity (also in WeaponSystem, but replicated for clarity)

enum class Rarity {
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    GodTier
};

// MARK: - Item Type

enum class ItemType {
    Weapon,
    Armor,
    Accessory,
    Consumable,
    Material,
    QuestItem,
    Rune,
    SpellBook,
    GodRuin
};

// MARK: - Item Definition

struct Item {
    std::string id;
    std::string name;
    ItemType type;
    Rarity rarity;
    int maxStackSize;
    int currentStack;
    float weight;
    std::string description;
    std::unordered_map<std::string, float> stats;  // Flexible stats
    std::string iconId;
    bool isEquippable;
    std::string requiredSlot;  // For equippable items
    
    Item(const std::string& itemId, const std::string& itemName,
         ItemType itemType, Rarity itemRarity = Rarity::Common,
         int maxStack = 1, const std::string& desc = "", float wgt = 1.0f)
        : id(itemId), name(itemName), type(itemType), rarity(itemRarity),
          maxStackSize(maxStack), currentStack(1), weight(wgt),
          description(desc), iconId("default"), isEquippable(false),
          requiredSlot("") {}
    
    bool canStackWith(const Item& other) const {
        return id == other.id && currentStack < maxStackSize;
    }
    
    float getTotalWeight() const {
        return weight * currentStack;
    }
};

// MARK: - Inventory Component (ECS)

struct InventoryComponent : public Component {
    std::vector<std::shared_ptr<Item>> items;
    int maxSlots;
    float maxWeight;
    float currentWeight;
    
    InventoryComponent(int slots = 20, float maxWgt = 100.0f)
        : maxSlots(slots), maxWeight(maxWgt), currentWeight(0.0f) {}
    
    bool addItem(std::shared_ptr<Item> item) {
        // Check weight
        if (currentWeight + item->getTotalWeight() > maxWeight) {
            return false;  // Overweight
        }
        
        // Try to stack with existing items
        if (item->maxStackSize > 1) {
            for (auto& existing : items) {
                if (existing->canStackWith(*item)) {
                    int canAdd = std::min(item->currentStack, existing->maxStackSize - existing->currentStack);
                    existing->currentStack += canAdd;
                    item->currentStack -= canAdd;
                    
                    if (item->currentStack <= 0) {
                        currentWeight += item->getTotalWeight();
                        return true;
                    }
                }
            }
        }
        
        // Add as new slot
        if (static_cast<int>(items.size()) >= maxSlots) {
            return false;  // Inventory full
        }
        
        items.push_back(item);
        currentWeight += item->getTotalWeight();
        return true;
    }
    
    bool removeItem(const std::string& itemId, int count = 1) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if ((*it)->id == itemId) {
                if ((*it)->currentStack > count) {
                    (*it)->currentStack -= count;
                    currentWeight -= (*it)->weight * count;
                } else {
                    currentWeight -= (*it)->getTotalWeight();
                    items.erase(it);
                }
                return true;
            }
        }
        return false;
    }
    
    std::shared_ptr<Item> getItem(const std::string& itemId) const {
        for (const auto& item : items) {
            if (item->id == itemId) {
                return item;
            }
        }
        return nullptr;
    }
    
    int getItemCount(const std::string& itemId) const {
        auto item = getItem(itemId);
        return item ? item->currentStack : 0;
    }
    
    float getTotalValue() const {
        // In a real implementation, would have item values
        return 0.0f;
    }
    
    int getUsedSlots() const {
        return static_cast<int>(items.size());
    }
    
    bool isFull() const {
        return static_cast<int>(items.size()) >= maxSlots;
    }
};

// MARK: - Equipment Component (ECS)

struct EquipmentComponent : public Component {
    std::unordered_map<std::string, std::shared_ptr<Item>> equipped;  // slot -> item
    
    EquipmentComponent() {}
    
    bool equipItem(std::shared_ptr<Item> item, EntityManager& entities, const Entity& player) {
        if (!item->isEquippable) return false;
        
        // Unequip existing item in that slot
        if (equipped.find(item->requiredSlot) != equipped.end()) {
            // Would add to inventory
        }
        
        equipped[item->requiredSlot] = item;
        
        // Apply item stats
        applyItemStats(item, entities, player, true);
        return true;
    }
    
    bool unequipItem(const std::string& slot, EntityManager& entities, const Entity& player) {
        auto it = equipped.find(slot);
        if (it == equipped.end()) return false;
        
        // Remove stats
        applyItemStats(it->second, entities, player, false);
        
        // Add to inventory
        auto inventory = entities.componentStorage.getComponent<InventoryComponent>(player);
        if (inventory) {
            inventory->addItem(it->second);
        }
        
        equipped.erase(slot);
        return true;
    }
    
    std::shared_ptr<Item> getEquippedItem(const std::string& slot) const {
        auto it = equipped.find(slot);
        if (it != equipped.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    void applyItemStats(std::shared_ptr<Item> item, EntityManager& entities,
                       const Entity& player, bool apply) {
        auto playerData = entities.componentStorage.getComponent<PlayerDataComponent>(player);
        if (!playerData) return;
        
        // Apply stats from item
        for (const auto& pair : item->stats) {
            float value = pair.second * (apply ? 1.0f : -1.0f);
            
            if (pair.first == "health_boost") {
                playerData->level.maxHealth += value;
            } else if (pair.first == "damage_boost") {
                // Would apply to weapon
            }
            // Add more stat applications
        }
        
        entities.componentStorage.addComponent<PlayerDataComponent>(player, playerData);
    }
};

// MARK: - Predefined Items

namespace Items {
    
    // Consumables
    static Item HealthPotion("item_health_potion", "Health Potion",
                             ItemType::Consumable, Rarity::Common, 5);
    
    static Item ManaPotion("item_mana_potion", "Mana Potion",
                            ItemType::Consumable, Rarity::Common, 5);
    
    // Materials
    static Item DragonScale("item_dragon_scale", "Dragon Scale",
                            ItemType::Material, Rarity::Rare, 10);
    
    static Item IronOre("item_iron_ore", "Iron Ore",
                          ItemType::Material, Rarity::Common, 20);
    
    // Runes
    static Item FireRuneItem("item_rune_fire", "Rune of Fire",
                             ItemType::Rune, Rarity::Uncommon);
    
    // Spell Books
    static Item FireballBook("item_spell_fireball", "Spell: Fireball",
                            ItemType::SpellBook, Rarity::Common);
    
    static std::vector<std::shared_ptr<Item>> AllItems = {
        std::make_shared<Item>(HealthPotion),
        std::make_shared<Item>(ManaPotion),
        std::make_shared<Item>(DragonScale),
        std::make_shared<Item>(IronOre),
        std::make_shared<Item>(FireRuneItem),
        std::make_shared<Item>(FireballBook)
    };
    
    static std::shared_ptr<Item> getItemById(const std::string& id) {
        for (const auto& item : AllItems) {
            if (item->id == id) return item;
        }
        return nullptr;
    }
}

// MARK: - Inventory System

class InventorySystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        // Process items that need updating (like consumables with durations)
        auto inventories = entities.componentStorage.getAllComponents<InventoryComponent>();
        
        for (auto& pair : inventories) {
            Entity entity = pair.first;
            auto inventory = pair.second;
            
            // Check for overweight and drop items if needed
            if (inventory->currentWeight > inventory->maxWeight) {
                // Would drop heaviest items
            }
        }
    }
    
    static bool giveItemToPlayer(const Entity& player, EntityManager& entities,
                                std::shared_ptr<Item> item) {
        auto inventory = entities.componentStorage.getComponent<InventoryComponent>(player);
        if (!inventory) {
            inventory = std::make_shared<InventoryComponent>();
            entities.componentStorage.addComponent<InventoryComponent>(player, inventory);
        }
        
        return inventory->addItem(item);
    }
    
    static bool useItem(const Entity& player, EntityManager& entities,
                       const std::string& itemId) {
        auto inventory = entities.componentStorage.getComponent<InventoryComponent>(player);
        if (!inventory) return false;
        
        auto item = inventory->getItem(itemId);
        if (!item || item->type != ItemType::Consumable) return false;
        
        // Apply consumable effect
        if (itemId == "item_health_potion") {
            auto playerData = entities.componentStorage.getComponent<PlayerDataComponent>(player);
            if (playerData) {
                playerData->level.currentXP += 50;  // Example effect
            }
        }
        
        // Remove from inventory
        inventory->removeItem(itemId);
        return true;
    }
};

} // namespace Doomnite
