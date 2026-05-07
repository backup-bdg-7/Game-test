// PlayerProgression.hpp - C++ Player Progression & Save System for Doomnite
// Handles player level, XP, unlocked content, achievements, and save/load

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Player Level/XP

struct PlayerLevel {
    int level;
    float currentXP;
    float xpToNextLevel;
    int skillPoints;  // Points to spend on skill tree
    
    PlayerLevel(int lvl = 1, float xp = 0.0f) : level(lvl), currentXP(xp), skillPoints(0) {
        calculateXPForNextLevel();
    }
    
    void addXP(float xp) {
        currentXP += xp;
        while (currentXP >= xpToNextLevel && level < 100) { // Max level 100
            levelUp();
        }
    }
    
    void levelUp() {
        level++;
        currentXP -= xpToNextLevel;
        skillPoints += 2;  // 2 skill points per level
        calculateXPForNextLevel();
    }
    
    void calculateXPForNextLevel() {
        // Exponential XP curve
        xpToNextLevel = 100.0f * powf(1.2f, static_cast<float>(level));
    }
    
    float getProgressToNextLevel() const {
        return currentXP / xpToNextLevel;
    }
};

// MARK: - Unlockable Content

enum class UnlockType {
    Weapon,
    Armor,
    Spell,
    Companion,
    Rune,
    GodRuin,
    Zone,
    Quest,
    Achievement
};

struct Unlockable {
    std::string id;
    std::string name;
    UnlockType type;
    int requiredLevel;
    std::string description;
    bool isUnlocked;
    
    Unlockable(const std::string& uId, const std::string& uName,
                UnlockType uType, int reqLevel = 1,
                const std::string& desc = "")
        : id(uId), name(uName), type(uType), requiredLevel(reqLevel),
          description(desc), isUnlocked(false) {}
};

// MARK: - Achievement

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    float progress;       // 0.0 to 1.0
    float targetValue;
    float currentValue;
    bool isCompleted;
    float rewardXP;
    
    Achievement(const std::string& achId, const std::string& achName,
                 const std::string& desc, float target, float xpReward = 100.0f)
        : id(achId), name(achName), description(desc), progress(0.0f),
          targetValue(target), currentValue(0.0f), isCompleted(false),
          rewardXP(xpReward) {}
    
    void updateProgress(float value) {
        currentValue += value;
        progress = fminf(currentValue / targetValue, 1.0f);
        
        if (progress >= 1.0f && !isCompleted) {
            isCompleted = true;
        }
    }
};

// MARK: - Player Profile

struct PlayerProfile {
    std::string username;
    std::string userId;  // For server sync
    std::string iconId;  // Selected icon
    std::string customPhotoPath;  // User's own photo
    time_t createdDate;
    time_t lastPlayed;
    
    PlayerProfile(const std::string& name = "") {
        username = name.empty() ? generateRandomUsername() : name;
        userId = generateUUID();
        iconId = "default_1";
        customPhotoPath = "";
        createdDate = time(nullptr);
        lastPlayed = time(nullptr);
    }
    
    static std::string generateRandomUsername() {
        std::vector<std::string> prefixes = {"Brave", "Mighty", "Epic", "Noble", "Shadow", "Fire", "Ice", "Storm"};
        std::vector<std::string> suffixes = {"Warrior", "Mage", "Hunter", "Knight", "Dragon", "Wolf", "Bear", "Eagle"};
        
        int pIdx = rand() % prefixes.size();
        int sIdx = rand() % suffixes.size();
        int num = rand() % 1000;
        
        return prefixes[pIdx] + suffixes[sIdx] + std::to_string(num);
    }
    
    static std::string generateUUID() {
        // Simple UUID generation (in production, use proper UUID library)
        char uuid[37];
        snprintf(uuid, sizeof(uuid), "%08x-%04x-%04x-%04x-%012x",
                 rand(), rand() & 0xffff, rand() & 0xffff,
                 rand() & 0xffff, rand() & 0xffffffffffff);
        return std::string(uuid);
    }
};

// MARK: - Player Data Component (ECS)

struct PlayerDataComponent : public Component {
    PlayerProfile profile;
    PlayerLevel level;
    std::unordered_map<std::string, std::shared_ptr<Unlockable>> unlockedContent;
    std::vector<std::shared_ptr<Achievement>> achievements;
    
    // Stats
    int totalKills;
    int totalDeaths;
    float totalPlayTime;
    int totalQuestsCompleted;
    
    // Settings
    bool showHints;
    bool enableTutorial;
    bool isOnlineMode;
    std::string serverUrl;
    
    PlayerDataComponent(const std::string& username = "")
        : profile(username), level(1, 0.0f), showHints(true),
          enableTutorial(true), isOnlineMode(false), totalKills(0),
          totalDeaths(0), totalPlayTime(0.0f), totalQuestsCompleted(0) {}
    
    bool isContentUnlocked(const std::string& contentId) const {
        return unlockedContent.find(contentId) != unlockedContent.end() &&
               unlockedContent.at(contentId)->isUnlocked;
    }
    
    void unlockContent(const std::string& contentId) {
        if (unlockedContent.find(contentId) != unlockedContent.end()) {
            unlockedContent[contentId]->isUnlocked = true;
        }
    }
    
    void addAchievement(std::shared_ptr<Achievement> achievement) {
        achievements.push_back(achievement);
    }
    
    std::shared_ptr<Achievement> getAchievement(const std::string& achievementId) {
        for (auto& ach : achievements) {
            if (ach->id == achievementId) return ach;
        }
        return nullptr;
    }
    
    void updatePlayTime(float deltaTime) {
        totalPlayTime += deltaTime;
        profile.lastPlayed = time(nullptr);
    }
};

// MARK: - Save/Load System

struct SaveGame {
    std::string username;
    int level;
    float xp;
    time_t saveTime;
    
    // Serialized data (in real impl, would use JSON, binary, or protobuf)
    std::string toString() const {
        // Simplified - would use proper serialization
        return "Save: " + username + " Level: " + std::to_string(level);
    }
};

class PlayerProgressionSystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
        auto players = entities.componentStorage.getAllComponents<PlayerDataComponent>();
        
        for (auto& pair : players) {
            Entity entity = pair.first;
            auto playerData = pair.second;
            
            // Update play time
            playerData->updatePlayTime(deltaTime);
            
            // Check for level-based unlocks
            checkLevelUnlocks(playerData);
            
            // Update achievements
            for (auto& achievement : playerData->achievements) {
                if (achievement->isCompleted) {
                    // Grant rewards if just completed
                }
            }
            
            // Save periodically (every 5 minutes)
            static float saveTimer = 0.0f;
            saveTimer += deltaTime;
            if (saveTimer >= 300.0f) {  // 5 minutes
                saveGame(entity, entities);
                saveTimer = 0.0f;
            }
            
            // Update back in storage
            entities.componentStorage.addComponent<PlayerDataComponent>(entity, playerData);
        }
    }
    
    void saveGame(const Entity& player, EntityManager& entities) {
        auto playerData = entities.componentStorage.getComponent<PlayerDataComponent>(player);
        if (!playerData) return;
        
        // In offline mode, save to local file
        if (!playerData->isOnlineMode) {
            saveToLocalFile(*playerData);
        } else {
            // In online mode, would sync to server
            // syncToServer(*playerData);
        }
    }
    
    bool loadGame(const std::string& username, EntityManager& entities, const Entity& player) {
        // Try local file first
        auto playerData = loadFromLocalFile(username);
        if (playerData) {
            entities.componentStorage.addComponent<PlayerDataComponent>(player, playerData);
            return true;
        }
        return false;
    }
    
private:
    void checkLevelUnlocks(std::shared_ptr<PlayerDataComponent> playerData) {
        // Check if new content should be unlocked based on level
        for (auto& pair : playerData->unlockedContent) {
            auto& unlockable = pair.second;
            if (!unlockable->isUnlocked && playerData->level.level >= unlockable->requiredLevel) {
                unlockable->isUnlocked = true;
                // Notify player
            }
        }
    }
    
    void saveToLocalFile(const PlayerDataComponent& playerData) {
        // In real implementation, would serialize to JSON and write to Documents directory
        // For iOS: use FileManager to get Documents directory
    }
    
    std::shared_ptr<PlayerDataComponent> loadFromLocalFile(const std::string& username) {
        // Load from local file
        return nullptr;  // Placeholder
    }
};

} // namespace Doomnite
