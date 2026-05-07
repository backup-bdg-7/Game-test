// PlayerProgression.cpp - C++ Player Progression Implementation

#include "PlayerProgression.hpp"
#include <algorithm>
#include <ctime>

namespace Doomnite {

// MARK: - PlayerLevel Implementation

void PlayerLevel::addXP(float xp) {
    currentXP += xp;
    while (currentXP >= xpToNextLevel && level < 100) {
        levelUp();
    }
}

void PlayerLevel::levelUp() {
    level++;
    currentXP -= xpToNextLevel;
    skillPoints += 2;  // 2 skill points per level
    calculateXPForNextLevel();
}

void PlayerLevel::calculateXPForNextLevel() {
    // Exponential XP curve
    xpToNextLevel = 100.0f * powf(1.2f, static_cast<float>(level));
}

float PlayerLevel::getProgressToNextLevel() const {
    return currentXP / xpToNextLevel;
}

// MARK: - PlayerProfile Implementation

PlayerProfile::PlayerProfile(const std::string& name) {
    username = name.empty() ? generateRandomUsername() : name;
    userId = generateUUID();
    iconId = "default_1";
    customPhotoPath = "";
    createdDate = time(nullptr);
    lastPlayed = time(nullptr);
}

std::string PlayerProfile::generateRandomUsername() {
    std::vector<std::string> prefixes = {"Brave", "Mighty", "Epic", "Noble", "Shadow", "Fire", "Storm"};
    std::vector<std::string> suffixes = {"Warrior", "Mage", "Hunter", "Knight", "Dragon", "Wolf", "Bear", "Eagle"};
    
    int pIdx = rand() % prefixes.size();
    int sIdx = rand() % suffixes.size();
    int num = rand() % 1000;
    
    return prefixes[pIdx] + suffixes[sIdx] + std::to_string(num);
}

std::string PlayerProfile::generateUUID() {
    // Simple UUID generation (in production, use proper UUID library)
    char uuid[37];
    snprintf(uuid, sizeof(uuid), "%08x-%04x-%04x-%04x-%012x",
             rand(), rand() & 0xffff, rand() & 0xffff,
             rand() & 0xffff, rand() & 0xffffffffffff);
    return std::string(uuid);
}

// MARK: - PlayerDataComponent Implementation

bool PlayerDataComponent::isContentUnlocked(const std::string& contentId) const {
    auto it = unlockedContent.find(contentId);
    return it != unlockedContent.end() && it->second->isUnlocked;
}

void PlayerDataComponent::unlockContent(const std::string& contentId) {
    auto it = unlockedContent.find(contentId);
    if (it != unlockedContent.end()) {
        it->second->isUnlocked = true;
    }
}

void PlayerDataComponent::addAchievement(std::shared_ptr<Achievement> achievement) {
    // Check if already exists
    for (const auto& ach : achievements) {
        if (ach->id == achievement->id) return;
    }
    achievements.push_back(achievement);
}

std::shared_ptr<Achievement> PlayerDataComponent::getAchievement(const std::string& achievementId) {
    for (auto& ach : achievements) {
        if (ach->id == achievementId) return ach;
    }
    return nullptr;
}

void PlayerDataComponent::updatePlayTime(float deltaTime) {
    totalPlayTime += deltaTime;
    profile.lastPlayed = time(nullptr);
}

// MARK: - PlayerProgressionSystem Implementation

void PlayerProgressionSystem::update(EntityManager& entities, float deltaTime) {
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
                // In full implementation, would check if just completed this frame
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

void PlayerProgressionSystem::saveGame(const Entity& player, EntityManager& entities) {
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

bool PlayerProgressionSystem::loadGame(const std::string& username, EntityManager& entities, const Entity& player) {
    // Try local file first
    auto playerData = loadFromLocalFile(username);
    if (playerData) {
        entities.componentStorage.addComponent<PlayerDataComponent>(player, playerData);
        return true;
    }
    return false;
}

void PlayerProgressionSystem::checkLevelUnlocks(std::shared_ptr<PlayerDataComponent> playerData) {
    // Check if new content should be unlocked based on level
    for (auto& pair : playerData->unlockedContent) {
        auto& unlockable = pair.second;
        if (!unlockable->isUnlocked && playerData->level.level >= unlockable->requiredLevel) {
            unlockable->isUnlocked = true;
            // Notify player
        }
    }
}

void PlayerProgressionSystem::saveToLocalFile(const PlayerDataComponent& playerData) {
    // In real implementation, would serialize to JSON and write to Documents directory
    // For iOS: use FileManager to get Documents directory
}

std::shared_ptr<PlayerDataComponent> PlayerProgressionSystem::loadFromLocalFile(const std::string& username) {
    // Load from local file
    return nullptr;  // Placeholder
}

} // namespace Doomnite
