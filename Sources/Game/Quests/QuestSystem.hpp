// QuestSystem.hpp - C++ Quest System for Doomnite
// Main story, side quests, daily quests with objectives and rewards

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Quest Objective

struct QuestObjective {
    std::string id;
    std::string description;
    ObjectiveType type;
    float currentValue;
    float targetValue;
    bool isCompleted;
    std::string targetId;  // Target entity/item ID
    
    QuestObjective(const std::string& objId, const std::string& desc,
                   ObjectiveType objType, float target, const std::string& target = "")
        : id(objId), description(desc), type(objType),
          currentValue(0.0f), targetValue(target), isCompleted(false), targetId(target) {}
    
    float getProgress() const {
        return currentValue / targetValue;
    }
    
    void updateProgress(float value) {
        currentValue += value;
        if (currentValue >= targetValue) {
            isCompleted = true;
        }
    }
};

enum class ObjectiveType {
    KillEnemies,
    CollectItems,
    ReachLocation,
    CompleteDungeon,
    LevelUp,
    EquipItem,
    LearnSpell,
    TameCompanion,
    UpgradeWeapon,
    TalkToNPC
};

// MARK: - Quest Reward

struct QuestReward {
    RewardType type;
    float amount;
    std::string itemId;  // For item rewards
    
    QuestReward(RewardType rewardType, float amt = 0.0f, const std::string& item = "")
        : type(rewardType), amount(amt), itemId(item) {}
};

enum class RewardType {
    XP,
    Gold,       // If we had currency
    Item,
    Weapon,
    Armor,
    Spell,
    Rune,
    Companion,
    SkillPoints
};

// MARK: - Quest Definition

struct Quest {
    std::string id;
    std::string name;
    std::string description;
    QuestType type;
    int requiredLevel;
    std::string previousQuestId;  // For quest chains
    std::vector<std::shared_ptr<QuestObjective>> objectives;
    std::vector<QuestReward> rewards;
    std::string questGiverNPC;
    std::string zoneId;
    bool isRepeatable;  // For daily/weekly quests
    
    Quest(const std::string& questId, const std::string& questName,
           const std::string& desc, QuestType questType = QuestType::MainStory,
           int reqLevel = 1, const std::string& prevQuest = "",
           bool repeatable = false)
        : id(questId), name(questName), description(desc), type(questType),
          requiredLevel(reqLevel), previousQuestId(prevQuest),
          isRepeatable(repeatable) {}
    
    bool areAllObjectivesComplete() const {
        for (const auto& obj : objectives) {
            if (!obj->isCompleted) return false;
        }
        return true;
    }
    
    float getOverallProgress() const {
        if (objectives.empty()) return 1.0f;
        
        float total = 0.0f;
        for (const auto& obj : objectives) {
            total += obj->getProgress();
        }
        return total / static_cast<float>(objectives.size());
    }
};

enum class QuestType {
    MainStory,
    SideQuest,
    DailyQuest,
    WeeklyQuest,
    Tutorial
};

// MARK: - Quest Component (ECS)

struct QuestComponent : public Component {
    std::vector<std::shared_ptr<Quest>> activeQuests;
    std::vector<std::string> completedQuestIds;
    std::vector<std::shared_ptr<Quest>> completedQuests;  // Full data for rewards
    
    QuestComponent() {}
    
    bool startQuest(std::shared_ptr<Quest> quest, EntityManager& entities, const Entity& player) {
        // Check if already started
        for (const auto& q : activeQuests) {
            if (q->id == quest->id) return false;
        }
        
        // Check level requirement
        auto playerData = entities.componentStorage.getComponent<PlayerDataComponent>(player);
        if (playerData && playerData->level.level < quest->requiredLevel) {
            return false;
        }
        
        // Check quest chain
        if (!quest->previousQuestId.empty()) {
            bool found = false;
            for (const auto& completedId : completedQuestIds) {
                if (completedId == quest->previousQuestId) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;  // Previous quest not completed
        }
        
        activeQuests.push_back(quest);
        return true;
    }
    
    bool completeQuest(const std::string& questId, EntityManager& entities, const Entity& player) {
        // Find quest
        std::shared_ptr<Quest> questToComplete = nullptr;
        int index = -1;
        
        for (int i = 0; i < static_cast<int>(activeQuests.size()); i++) {
            if (activeQuests[i]->id == questId) {
                questToComplete = activeQuests[i];
                index = i;
                break;
            }
        }
        
        if (!questToComplete || !questToComplete->areAllObjectivesComplete()) {
            return false;
        }
        
        // Grant rewards
        grantRewards(questToComplete, entities, player);
        
        // Move to completed
        completedQuestIds.push_back(questId);
        completedQuests.push_back(questToComplete);
        activeQuests.erase(activeQuests.begin() + index);
        
        return true;
    }
    
    void grantRewards(std::shared_ptr<Quest> quest, EntityManager& entities, const Entity& player) {
        auto playerData = entities.componentStorage.getComponent<PlayerDataComponent>(player);
        if (!playerData) return;
        
        for (const auto& reward : quest->rewards) {
            switch (reward.type) {
                case RewardType::XP:
                    playerData->level.addXP(reward.amount);
                    break;
                case RewardType::SkillPoints:
                    // Would add skill points
                    break;
                case RewardType::Item:
                case RewardType::Weapon:
                case RewardType::Armor:
                    // Would add to inventory
                    break;
                default:
                    break;
            }
        }
        
        // Update player stats
        playerData->totalQuestsCompleted++;
        
        entities.componentStorage.addComponent<PlayerDataComponent>(player, playerData);
    }
    
    std::shared_ptr<Quest> getActiveQuest(const std::string& questId) const {
        for (const auto& quest : activeQuests) {
            if (quest->id == questId) return quest;
        }
        return nullptr;
    }
    
    bool isQuestCompleted(const std::string& questId) const {
        for (const auto& completedId : completedQuestIds) {
            if (completedId == questId) return true;
        }
        return false;
    }
};

// MARK: - Predefined Quests (From PLAN.MD)

namespace Quests {
    
    // MARK: Tutorial Quests
    static std::shared_ptr<Quest> tutorialCombat = std::make_shared<Quest>(
        "tutorial_combat", "Combat Basics", "Learn to fight enemies",
        QuestType::Tutorial, 1, "", false
    );
    
    // MARK: Main Story: "The God Ruin Prophecy" (15 quests, 5 chapters)
    static std::shared_ptr<Quest> chapter1DiscoverRuin = std::make_shared<Quest>(
        "main_ch1_discover_ruin", "Discover First Ruin",
        "Find your first God Ruin and learn about ancient power",
        QuestType::MainStory, 1, "", false
    );
    
    static std::shared_ptr<Quest> chapter2TameCompanion = std::make_shared<Quest>(
        "main_ch2_tame_companion", "Tame Your First Companion",
        "Learn to bond with animal companions",
        QuestType::MainStory, 5, "main_ch1_discover_ruin", false
    );
    
    static std::shared_ptr<Quest> chapter3DefeatGuardians = std::make_shared<Quest>(
        "main_ch3_defeat_guardians", "Defeat Ruin Guardians",
        "Collect 3 minor ruins from defeated guardians",
        QuestType::MainStory, 10, "main_ch2_tame_companion", false
    );
    
    static std::shared_ptr<Quest> chapter4UpgradeWeapons = std::make_shared<Quest>(
        "main_ch4_upgrade_weapons", "Upgrade Weapons with Runes",
        "Learn to socket runes and upgrade at blacksmith",
        QuestType::MainStory, 15, "main_ch3_defeat_guardians", false
    );
    
    static std::shared_ptr<Quest> chapter5FinalBattle = std::make_shared<Quest>(
        "main_ch5_final_battle", "Claim God-Tier Ruin",
        "Face the final challenge and claim your God-Tier ruin",
        QuestType::MainStory, 20, "main_ch4_upgrade_weapons", false
    );
    
    // MARK: Side Quests
    static std::shared_ptr<Quest> blacksmithRequest = std::make_shared<Quest>(
        "side_blacksmith_request", "The Blacksmith's Request",
        "Collect 10 dragon scales for the blacksmith",
        QuestType::SideQuest, 5, "", false
    );
    
    static std::shared_ptr<Quest> spellMerchantApprentice = std::make_shared<Quest>(
        "side_spell_merchant", "Spell Merchant's Apprentice",
        "Learn 5 spells and unlock enchanting",
        QuestType::SideQuest, 8, "", false
    );
    
    static std::shared_ptr<Quest> dragonTamer = std::make_shared<Quest>(
        "side_dragon_tamer", "Dragon Tamer",
        "Find and tame all 3 dragon types",
        QuestType::SideQuest, 15, "", false
    );
    
    static std::shared_ptr<Quest> runeHunter = std::make_shared<Quest>(
        "side_rune_hunter", "Rune Hunter",
        "Collect all 7 rune types and unlock set bonus",
        QuestType::SideQuest, 12, "", false
    );
    
    // MARK: Daily Quests
    static std::shared_ptr<Quest> dailyKill10 = std::make_shared<Quest>(
        "daily_kill_10", "Daily: Kill 10 Enemies",
        "Defeat 10 enemies to earn XP and loot",
        QuestType::DailyQuest, 1, "", true
    );
    
    static std::shared_ptr<Quest> dailyCollect5 = std::make_shared<Quest>(
        "daily_collect_5", "Daily: Collect 5 Items",
        "Find and collect 5 items in the world",
        QuestType::DailyQuest, 1, "", true
    );
    
    static std::vector<std::shared_ptr<Quest>> AllQuests = {
        tutorialCombat,
        chapter1DiscoverRuin, chapter2TameCompanion, chapter3DefeatGuardians,
        chapter4UpgradeWeapons, chapter5FinalBattle,
        blacksmithRequest, spellMerchantApprentice, dragonTamer, runeHunter,
        dailyKill10, dailyCollect5
    };
    
    static std::shared_ptr<Quest> getQuestById(const std::string& id) {
        for (const auto& quest : AllQuests) {
            if (quest->id == id) return quest;
        }
        return nullptr;
    }
}

// MARK: - Quest System

class QuestSystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        auto questComponents = entities.componentStorage.getAllComponents<QuestComponent>();
        
        for (auto& pair : questComponents) {
            Entity entity = pair.first;
            auto questComp = pair.second;
            
            // Check for completed quests
            std::vector<std::shared_ptr<Quest>> toComplete;
            for (const auto& quest : questComp->activeQuests) {
                if (quest->areAllObjectivesComplete()) {
                    toComplete.push_back(quest);
                }
            }
            
            // Complete quests
            for (const auto& quest : toComplete) {
                questComp->completeQuest(quest->id, entities, entity);
            }
            
            // Update back in storage
            entities.componentStorage.addComponent<QuestComponent>(entity, questComp);
        }
    }
    
    // Helper to update quest objectives (called from other systems)
    static void updateObjective(Entity player, const std::string& objectiveId,
                                  float progress, EntityManager& entities) {
        auto questComp = entities.componentStorage.getComponent<QuestComponent>(player);
        if (!questComp) return;
        
        for (const auto& quest : questComp->activeQuests) {
            for (const auto& obj : quest->objectives) {
                if (obj->id == objectiveId) {
                    obj->updateProgress(progress);
                }
            }
        }
    }
    
    // Check if player can start a quest
    static bool canStartQuest(const Entity& player, std::shared_ptr<Quest> quest,
                               EntityManager& entities) {
        auto questComp = entities.componentStorage.getComponent<QuestComponent>(player);
        if (!questComp) return false;
        
        // Check if already completed (and not repeatable)
        if (questComp->isQuestCompleted(quest->id) && !quest->isRepeatable) {
            return false;
        }
        
        // Check level
        auto playerData = entities.componentStorage.getComponent<PlayerDataComponent>(player);
        if (playerData && playerData->level.level < quest->requiredLevel) {
            return false;
        }
        
        return true;
    }
};

} // namespace Doomnite
