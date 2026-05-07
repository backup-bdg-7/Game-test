// AudioSystem.hpp - C++ Audio System for Doomnite
// Handles music, SFX, 3D positional audio, and haptic feedback

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Audio Type

enum class AudioType {
    Music,
    SFX,        // Sound effects
    Dialogue,
    Ambient,
    Haptic      // Haptic feedback (iOS-specific)
};

// MARK: - Audio Clip

struct AudioClip {
    std::string id;
    std::string filename;
    AudioType type;
    float volume;
    float pitch;
    bool isLooping;
    bool is3D;        // Positional audio
    float minDistance;  // For 3D audio
    float maxDistance;
    
    AudioClip(const std::string& clipId, const std::string& file,
              AudioType audioType = AudioType::SFX, float vol = 1.0f,
              bool looping = false, bool is3D = false)
        : id(clipId), filename(file), type(audioType), volume(vol),
          pitch(1.0f), isLooping(looping), is3D(is3D),
          minDistance(1.0f), maxDistance(50.0f) {}
};

// MARK: - Audio Source Component (ECS)

struct AudioSourceComponent : public Component {
    std::string currentClipId;
    bool isPlaying;
    bool is3D;
    Vec3 worldPosition;
    float volume;
    float pitch;
    bool isLooping;
    
    AudioSourceComponent(const std::string& clipId = "", bool playing = false,
                         bool is3D = false, float vol = 1.0f)
        : currentClipId(clipId), isPlaying(playing), is3D(is3D),
          volume(vol), pitch(1.0f), isLooping(false) {}
    
    void play(const std::string& clipId) {
        currentClipId = clipId;
        isPlaying = true;
    }
    
    void stop() {
        isPlaying = false;
    }
    
    void setPosition(const Vec3& position) {
        worldPosition = position;
        is3D = true;
    }
};

// MARK: - Haptic Feedback (iOS-specific)

struct HapticFeedback {
    std::string id;
    std::string description;
    float intensity;      // 0.0 to 1.0
    float sharpness;      // 0.0 to 1.0
    float duration;       // seconds
    
    HapticFeedback(const std::string& hapticId, const std::string& desc,
                   float inten = 0.5f, float sharp = 0.5f, float dur = 0.1f)
        : id(hapticId), description(desc), intensity(inten),
          sharpness(sharp), duration(dur) {}
};

// MARK: - Predefined Audio Clips

namespace AudioClips {
    // Weapon sounds
    static AudioClip pistolFire("sfx_pistol_fire", "weapons/pistol_fire.wav", AudioType::SFX, 0.8f);
    static AudioClip rifleFire("sfx_rifle_fire", "weapons/rifle_fire.wav", AudioType::SFX, 0.9f);
    static AudioClip swordSwing("sfx_sword_swing", "weapons/sword_swing.wav", AudioType::SFX, 0.6f);
    
    // Spell sounds
    static AudioClip fireballCast("sfx_fireball_cast", "spells/fireball_cast.wav", AudioType::SFX, 1.0f);
    static AudioClip lightningBolt("sfx_lightning_bolt", "spells/lightning_bolt.wav", AudioType::SFX, 0.9f);
    
    // UI sounds
    static AudioClip buttonClick("sfx_button_click", "ui/button_click.wav", AudioType::SFX, 0.5f);
    static AudioClip achievementUnlock("sfx_achievement", "ui/achievement.wav", AudioType::SFX, 0.8f);
    
    // Music
    static AudioClip mainTheme("music_main_theme", "music/main_theme.mp3", AudioType::Music, 0.7f, true);
    static AudioClip battleMusic("music_battle", "music/battle.mp3", AudioType::Music, 0.8f, true);
    
    static std::vector<AudioClip> AllClips = {
        pistolFire, rifleFire, swordSwing,
        fireballCast, lightningBolt,
        buttonClick, achievementUnlock,
        mainTheme, battleMusic
    };
}

// MARK: - Predefined Haptics (iOS)

namespace Haptics {
    static HapticFeedback lightImpact("haptic_light", "Light impact", 0.3f, 0.5f, 0.05f);
    static HapticFeedback mediumImpact("haptic_medium", "Medium impact", 0.6f, 0.7f, 0.1f);
    static HapticFeedback heavyImpact("haptic_heavy", "Heavy impact", 1.0f, 1.0f, 0.2f);
    static HapticFeedback swordHit("haptic_sword_hit", "Sword hit", 0.8f, 0.9f, 0.15f);
    static HapticFeedback spellCast("haptic_spell_cast", "Spell cast", 0.5f, 0.8f, 0.1f);
    
    static std::vector<HapticFeedback> AllHaptics = {
        lightImpact, mediumImpact, heavyImpact, swordHit, spellCast
    };
}

// MARK: - Audio System

class AudioSystem : public System {
private:
    std::unordered_map<std::string, AudioClip> audioClips;
    std::vector<AudioSourceComponent> activeSources;
    
public:
    AudioSystem() {
        // Load predefined clips
        for (const auto& clip : AudioClips::AllClips) {
            audioClips[clip.id] = clip;
        }
    }
    
    void update(EntityManager& entities, float deltaTime) override {
        auto audioSources = entities.componentStorage.getAllComponents<AudioSourceComponent>();
        
        for (auto& pair : audioSources) {
            Entity entity = pair.first;
            auto audioSource = pair.second;
            
            if (!audioSource->isPlaying) continue;
            
            // Update 3D audio position if needed
            if (audioSource->is3D) {
                auto transform = entities.componentStorage.getComponent<TransformComponent>(entity);
                if (transform) {
                    audioSource->worldPosition = transform->position;
                    // Would update audio engine with new position
                }
            }
            
            // In real implementation, would:
            // 1. Check if clip exists
            // 2. Play/loop audio
            // 3. Update 3D audio based on camera position
        }
    }
    
    void playSound(const std::string& clipId, EntityManager& entities, const Entity& entity) {
        auto it = audioClips.find(clipId);
        if (it == audioClips.end()) return;
        
        auto audioSource = entities.componentStorage.getComponent<AudioSourceComponent>(entity);
        if (!audioSource) {
            audioSource = std::make_shared<AudioSourceComponent>();
            entities.componentStorage.addComponent<AudioSourceComponent>(entity, audioSource);
        }
        
        audioSource->play(clipId);
    }
    
    void playHaptic(const std::string& hapticId, const Entity& player) {
        // On iOS, would trigger haptic feedback via Core Haptics
        // Would call iOS-specific code through Objective-C++ bridge
        // Example: HapticFeedbackManager.triggerHaptic(hapticId)
    }
    
    void play3DAudio(const std::string& clipId, const Vec3& position,
                        EntityManager& entities, const Entity& entity) {
        auto audioSource = entities.componentStorage.getComponent<AudioSourceComponent>(entity);
        if (!audioSource) {
            audioSource = std::make_shared<AudioSourceComponent>();
            audioSource->setPosition(position);
            entities.componentStorage.addComponent<AudioSourceComponent>(entity, audioSource);
        }
        
        audioSource->play(clipId);
    }
};

} // namespace Doomnite
