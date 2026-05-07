// SpellSystem.cpp - C++ Spell System Implementation

#include "SpellSystem.hpp"
#include <algorithm>

namespace Doomnite {

// MARK: - Spell Implementation

// Helper to create spell effects
static std::vector<SpellEffect> createFireballEffects() {
    return {
        SpellEffect(EffectType::FireDamage, 30.0f),
        SpellEffect(EffectType::FireDamage, 15.0f, 0.0f, 3.0f)
    };
}

Spell DestructionSpells::Fireball() {
    return Spell("destruction_fireball", "Fireball",
                 "Basic fire projectile that explodes on impact",
                 SpellSchool::Destruction, SpellType::Projectile,
                 15.0f, 1.5f, createFireballEffects(),
                 20.0f, true);
}

Spell DestructionSpells::LightningBolt() {
    std::vector<SpellEffect> effects = {
        SpellEffect(EffectType::ShockDamage, 40.0f),
        SpellEffect(EffectType::ShockDamage, 20.0f, 0.0f, 5.0f)
    };
    
    return Spell("destruction_lightning_bolt", "Lightning Bolt",
                 "Instant lightning damage that chains to nearby enemies",
                 SpellSchool::Destruction, SpellType::Projectile,
                 20.0f, 2.0f, effects, 100.0f, true);
}

Spell DestructionSpells::IceSpike() {
    std::vector<SpellEffect> effects = {
        SpellEffect(EffectType::FrostDamage, 25.0f),
        SpellEffect(EffectType::Slow, 0.5f, 3.0f)
    };
    
    return Spell("destruction_ice_spike", "Ice Spike",
                 "Projectile that deals frost damage and slows enemy",
                 SpellSchool::Destruction, SpellType::Projectile,
                 18.0f, 1.8f, effects, 25.0f, true);
}

Spell DestructionSpells::ChainLightning() {
    std::vector<SpellEffect> effects = {
        SpellEffect(EffectType::ShockDamage, 35.0f),
        SpellEffect(EffectType::ShockDamage, 25.0f, 0.0f, 8.0f)
    };
    
    return Spell("destruction_chain_lightning", "Chain Lightning",
                 "Hits 5 enemies in sequence with lightning damage",
                 SpellSchool::Destruction, SpellType::Projectile,
                 50.0f, 5.0f, effects, 80.0f, true);
}

Spell DestructionSpells::MeteorStorm() {
    std::vector<SpellEffect> effects = {
        SpellEffect(EffectType::FireDamage, 80.0f, 0.0f, 5.0f),
        SpellEffect(EffectType::FireDamage, 40.0f, 2.0f, 5.0f)
    };
    
    return Spell("destruction_meteor_storm", "Meteor Storm",
                 "Master spell that rains meteors for 10 seconds",
                 SpellSchool::Destruction, SpellType::AOE,
                 150.0f, 30.0f, effects, 0.0f, false, true);
}

// MARK: - ActiveSpellComponent Implementation

bool ActiveSpellComponent::canCast(const Spell& spell) const {
    // Check if spell is known
    if (std::find(knownSpells.begin(), knownSpells.end(), spell.id) == knownSpells.end()) {
        return false;
    }
    
    if (currentMana < spell.manaCost) return false;
    
    auto it = cooldownTimers.find(spell.id);
    if (it != cooldownTimers.end() && it->second > 0.0f) return false;
    
    return true;
}

bool ActiveSpellComponent::castSpell(const Spell& spell) {
    if (!canCast(spell)) return false;
    
    currentMana -= spell.manaCost;
    cooldownTimers[spell.id] = spell.cooldown;
    activeSpell = const_cast<Spell*>(&spell);  // In real impl, would copy
    return true;
}

void ActiveSpellComponent::updateCooldowns(float deltaTime) {
    for (auto& pair : cooldownTimers) {
        if (pair.second > 0.0f) {
            pair.second = std::max(0.0f, pair.second - deltaTime);
        }
    }
}

// MARK: - SpellCastingSystem Implementation

void SpellCastingSystem::update(EntityManager& entities, float deltaTime) {
    auto casters = entities.componentStorage.getAllComponents<ActiveSpellComponent>();
    
    for (auto& pair : casters) {
        Entity entity = pair.first;
        auto caster = pair.second;
        
        // Update cooldowns
        caster->updateCooldowns(deltaTime);
        
        // Process active spell if any
        if (caster->activeSpell) {
            // In a real implementation, this would:
            // 1. Create projectile entity for projectile spells
            // 2. Apply effects for self-cast spells
            // 3. Create AOE entity for AOE spells
            // 4. Spawn summoned creature for summon spells
            
            // For now, just clear active spell after "cast"
            // (Real implementation would have casting time/animation)
        }
        
        // Update back in storage
        entities.componentStorage.addComponent<ActiveSpellComponent>(entity, caster);
    }
}

} // namespace Doomnite
