// SpellSystem.hpp - C++ Spell System for Doomnite
// Elder Scrolls-style magic with 4 schools: Destruction, Restoration, Conjuration, Illusion

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Spell Schools

enum class SpellSchool {
    Destruction,
    Restoration,
    Conjuration,
    Illusion
};

// MARK: - Spell Type

enum class SpellType {
    Projectile,
    SelfCast,
    Touch,
    AOE,
    Summon,
    Enchantment
};

// MARK: - Effect Type

enum class EffectType {
    // Damage types
    FireDamage,
    FrostDamage,
    ShockDamage,
    PoisonDamage,
    MagicDamage,
    PhysicalDamage,
    
    // Healing
    RestoreHealth,
    RestoreMana,
    RestoreStamina,
    
    // Status effects
    Slow,
    Paralyze,
    Fear,
    Charm,
    Invisibility,
    
    // Summoning
    SummonCreature,
    
    // Other
    DispelMagic,
    SoulTrap
};

// MARK: - Spell Effect

struct SpellEffect {
    EffectType type;
    float magnitude;
    float duration;  // 0 for instant
    float radius;     // For AOE
    
    SpellEffect(EffectType effectType, float mag, float dur = 0.0f, float rad = 0.0f)
        : type(effectType), magnitude(mag), duration(dur), radius(rad) {}
};

// MARK: - Spell Data

struct Spell {
    std::string id;
    std::string name;
    std::string description;
    SpellSchool school;
    SpellType type;
    float manaCost;
    float cooldown;
    std::vector<SpellEffect> effects;
    float projectileSpeed;  // 0 for non-projectile
    bool requiresTarget;
    bool isMasterSpell;
    
    Spell(const std::string& spellId, const std::string& spellName,
          const std::string& desc, SpellSchool spellSchool, SpellType spellType,
          float cost, float cd, const std::vector<SpellEffect>& effs,
          float projSpeed = 0.0f, bool needsTarget = false, bool master = false)
        : id(spellId), name(spellName), description(desc), school(spellSchool),
          type(spellType), manaCost(cost), cooldown(cd), effects(effs),
          projectileSpeed(projSpeed), requiresTarget(needsTarget), isMasterSpell(master) {}
};

// MARK: - Destruction Spells

struct DestructionSpells {
    static Spell Fireball() {
        return Spell("destruction_fireball", "Fireball",
                     "Basic fire projectile that explodes on impact",
                     SpellSchool::Destruction, SpellType::Projectile,
                     15.0f, 1.5f, {
                         SpellEffect(EffectType::FireDamage, 30.0f),
                         SpellEffect(EffectType::FireDamage, 15.0f, 0.0f, 3.0f)
                     }, 20.0f, true);
    }
    
    static Spell LightningBolt() {
        return Spell("destruction_lightning_bolt", "Lightning Bolt",
                     "Instant lightning damage that chains to nearby enemies",
                     SpellSchool::Destruction, SpellType::Projectile,
                     20.0f, 2.0f, {
                         SpellEffect(EffectType::ShockDamage, 40.0f),
                         SpellEffect(EffectType::ShockDamage, 20.0f, 0.0f, 5.0f)
                     }, 100.0f, true);
    }
    
    static Spell IceSpike() {
        return Spell("destruction_ice_spike", "Ice Spike",
                     "Projectile that deals frost damage and slows enemy",
                     SpellSchool::Destruction, SpellType::Projectile,
                     18.0f, 1.8f, {
                         SpellEffect(EffectType::FrostDamage, 25.0f),
                         SpellEffect(EffectType::Slow, 0.5f, 3.0f)
                     }, 25.0f, true);
    }
    
    static Spell ChainLightning() {
        return Spell("destruction_chain_lightning", "Chain Lightning",
                     "Hits 5 enemies in sequence with lightning damage",
                     SpellSchool::Destruction, SpellType::Projectile,
                     50.0f, 5.0f, {
                         SpellEffect(EffectType::ShockDamage, 35.0f),
                         SpellEffect(EffectType::ShockDamage, 25.0f, 0.0f, 8.0f)
                     }, 80.0f, true);
    }
    
    static Spell MeteorStorm() {
        return Spell("destruction_meteor_storm", "Meteor Storm",
                     "Master spell that rains meteors for 10 seconds",
                     SpellSchool::Destruction, SpellType::AOE,
                     150.0f, 30.0f, {
                         SpellEffect(EffectType::FireDamage, 80.0f, 0.0f, 5.0f),
                         SpellEffect(EffectType::FireDamage, 40.0f, 2.0f, 5.0f)
                     }, 0.0f, false, true);
    }
};

// MARK: - Restoration Spells

struct RestorationSpells {
    static Spell Heal() {
        return Spell("restoration_heal", "Heal",
                     "Restore 50% HP over 3 seconds",
                     SpellSchool::Restoration, SpellType::SelfCast,
                     25.0f, 5.0f, {
                         SpellEffect(EffectType::RestoreHealth, 50.0f, 3.0f)
                     });
    }
    
    static Spell CurePoison() {
        return Spell("restoration_cure_poison", "Cure Poison",
                     "Remove all poison effects from target",
                     SpellSchool::Restoration, SpellType::SelfCast,
                     15.0f, 3.0f, {
                         SpellEffect(EffectType::PoisonDamage, -100.0f) // Negative = cure
                     });
    }
    
    static Spell RestoreMana() {
        return Spell("restoration_restore_mana", "Restore Mana",
                     "Instantly refill mana pool",
                     SpellSchool::Restoration, SpellType::SelfCast,
                     0.0f, 10.0f, {
                         SpellEffect(EffectType::RestoreMana, 100.0f)
                     });
    }
    
    static Spell Resurrection() {
        return Spell("restoration_resurrection", "Resurrection",
                     "Revive a fallen companion with 30 minute cooldown",
                     SpellSchool::Restoration, SpellType::Touch,
                     100.0f, 1800.0f, { // 30 minutes
                         SpellEffect(EffectType::RestoreHealth, 50.0f)
                     }, 0.0f, true);
    }
};

// MARK: - Conjuration Spells

struct ConjurationSpells {
    static Spell SummonWolf() {
        return Spell("conjuration_summon_wolf", "Summon Wolf",
                     "Temporary wolf companion for 60 seconds",
                     SpellSchool::Conjuration, SpellType::Summon,
                     40.0f, 60.0f, {
                         SpellEffect(EffectType::SummonCreature, 1.0f, 60.0f)
                     });
    }
    
    static Spell SummonBear() {
        return Spell("conjuration_summon_bear", "Summon Bear",
                     "Tank companion with high HP for 60 seconds",
                     SpellSchool::Conjuration, SpellType::Summon,
                     60.0f, 60.0f, {
                         SpellEffect(EffectType::SummonCreature, 1.0f, 60.0f)
                     });
    }
    
    static Spell SummonDragon() {
        return Spell("conjuration_summon_dragon", "Summon Dragon",
                     "Ultimate spell - dragon ally for 30 seconds",
                     SpellSchool::Conjuration, SpellType::Summon,
                     150.0f, 300.0f, { // 5 minutes
                         SpellEffect(EffectType::SummonCreature, 1.0f, 30.0f)
                     }, 0.0f, false, true);
    }
    
    static Spell BoundSword() {
        return Spell("conjuration_bound_sword", "Bound Sword",
                     "Conjure a magical sword with +100% damage for 60 seconds",
                     SpellSchool::Conjuration, SpellType::SelfCast,
                     50.0f, 60.0f, {
                         SpellEffect(EffectType::MagicDamage, 100.0f, 60.0f)
                     });
    }
};

// MARK: - Illusion Spells

struct IllusionSpells {
    static Spell Invisibility() {
        return Spell("illusion_invisibility", "Invisibility",
                     "Become invisible for 30 seconds (breaks on attack)",
                     SpellSchool::Illusion, SpellType::SelfCast,
                     50.0f, 60.0f, {
                         SpellEffect(EffectType::Invisibility, 1.0f, 30.0f)
                     });
    }
    
    static Spell CharmEnemy() {
        return Spell("illusion_charm_enemy", "Charm Enemy",
                     "Enemy fights for you for 60 seconds",
                     SpellSchool::Illusion, SpellType::Touch,
                     40.0f, 60.0f, {
                         SpellEffect(EffectType::Charm, 1.0f, 60.0f)
                     }, 0.0f, true);
    }
    
    static Spell FearAOE() {
        return Spell("illusion_fear_aoe", "Fear AOE",
                     "All nearby enemies flee in terror for 10 seconds",
                     SpellSchool::Illusion, SpellType::AOE,
                     45.0f, 30.0f, {
                         SpellEffect(EffectType::Fear, 1.0f, 10.0f, 10.0f)
                     });
    }
    
    static Spell Paralyze() {
        return Spell("illusion_paralyze", "Paralyze",
                     "Single enemy frozen in place for 5 seconds",
                     SpellSchool::Illusion, SpellType::Touch,
                     35.0f, 15.0f, {
                         SpellEffect(EffectType::Paralyze, 1.0f, 5.0f)
                     }, 0.0f, true);
    }
};

// MARK: - Active Spell Component (ECS)

struct ActiveSpellComponent : public Component {
    std::vector<std::string> knownSpells;  // Spell IDs
    float currentMana;
    float maxMana;
    Spell* activeSpell;
    std::unordered_map<std::string, float> cooldownTimers;  // Spell ID -> remaining cooldown
    
    ActiveSpellComponent(float mana = 100.0f, float max = 100.0f)
        : currentMana(mana), maxMana(max), activeSpell(nullptr) {}
    
    bool canCast(const Spell& spell) const {
        return std::find(knownSpells.begin(), knownSpells.end(), spell.id) != knownSpells.end() &&
               currentMana >= spell.manaCost &&
               cooldownTimers.at(spell.id) <= 0.0f;
    }
    
    bool castSpell(const Spell& spell) {
        if (!canCast(spell)) return false;
        
        currentMana -= spell.manaCost;
        cooldownTimers[spell.id] = spell.cooldown;
        activeSpell = const_cast<Spell*>(&spell);  // In real impl, would copy or reference
        return true;
    }
    
    void updateCooldowns(float deltaTime) {
        for (auto& pair : cooldownTimers) {
            pair.second = std::max(0.0f, pair.second - deltaTime);
        }
    }
};

// MARK: - Spell Casting System

class SpellCastingSystem : public System {
public:
    void update(EntityManager& entities, float deltaTime) override {
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
};

} // namespace Doomnite
