// ParticleSystem.hpp - C++ Particle System for Doomnite Metal Renderer
// GPU compute-based particles with spawners, emitters, and effects.

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ECS.hpp"

namespace Doomnite {

// MARK: - Particle Type

enum class ParticleType {
    Fire,
    Smoke,
    Sparks,
    Magic,
    Blood,
    Water,
    Snow,
    Dust,
    Explosion
};

// MARK: - Particle Emitter Component (ECS)

struct ParticleEmitterComponent : public Component {
    std::string particleSystemId;
    ParticleType type;
    Vec3 spawnPosition;
    Vec3 spawnArea;        // Random offset from spawn position
    float spawnRate;       // Particles per second
    float lifetime;        // Particle lifetime in seconds
    float speed;
    float speedVariation;
    float startSize;
    float endSize;
    Vec4 startColor;
    Vec4 endColor;
    float gravity;
    int maxParticles;
    bool isEmitting;
    float spawnTimer;
    
    ParticleEmitterComponent(const std::string& systemId = "", 
                           ParticleType particleType = ParticleType::Fire)
        : particleSystemId(systemId), type(particleType),
          spawnPosition(Vec3Zero), spawnArea(Vec3Zero),
          spawnRate(50.0f), lifetime(2.0f), speed(5.0f),
          speedVariation(0.5f), startSize(0.5f), endSize(0.1f),
          startColor(Vec4(1, 1, 1, 1)), endColor(Vec4(1, 1, 1, 0)),
          gravity(-9.81f), maxParticles(100), isEmitting(true), spawnTimer(0.0f) {}
    
    bool shouldSpawn(float deltaTime) {
        if (!isEmitting) return false;
        
        spawnTimer += deltaTime;
        float interval = 1.0f / spawnRate;
        
        if (spawnTimer >= interval) {
            spawnTimer -= interval;
            return true;
        }
        return false;
    }
};

// MARK: - Particle Structure (for GPU buffer)

struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec4 color;          // RGBA
    float size;
    float lifetime;        // Total lifetime
    float age;             // Current age
    float pad[2];        // Padding for alignment
};

// MARK: - Particle System Definition

struct ParticleSystem {
    std::string id;
    std::string name;
    ParticleType type;
    std::string textureId;     // Particle texture
    int maxParticles;
    bool isAdditive;       // Additive blending for fire/glow
    bool isSoft;           // Soft particles (depth fade)
    float depthFadeDistance;
    
    ParticleSystem(const std::string& systemId = "", const std::string& systemName = "",
                 ParticleType particleType = ParticleType::Fire)
        : id(systemId), name(systemName), type(particleType),
          textureId("particle_default"), maxParticles(100),
          isAdditive(false), isSoft(false), depthFadeDistance(1.0f) {}
};

// MARK: - Predefined Particle Systems

namespace ParticleSystems {
    
    // Fire effects
    static ParticleSystem Fire("particles_fire", "Fire Particles", ParticleType::Fire);
    static ParticleSystem Explosion("particles_explosion", "Explosion", ParticleType::Explosion);
    static ParticleSystem FireballTrail("particles_fireball_trail", "Fireball Trail", ParticleType::Magic);
    
    // Magic effects
    static ParticleSystem LightningSpark("particles_lightning_spark", "Lightning Sparks", ParticleType::Sparks);
    static ParticleSystem IceMist("particles_ice_mist", "Ice Mist", ParticleType::Magic);
    
    // Combat effects
    static ParticleSystem BloodSpurt("particles_blood", "Blood Spurt", ParticleType::Blood);
    static ParticleSystem SwordSwing("particles_sword_swing", "Sword Swing", ParticleType::Magic);
    
    // Environmental
    static ParticleSystem RainSplash("particles_rain_splash", "Rain Splash", ParticleType::Water);
    static ParticleSystem DustCloud("particles_dust", "Dust Cloud", ParticleType::Dust);
    
    static std::vector<ParticleSystem> AllSystems = {
        Fire, Explosion, FireballTrail,
        LightningSpark, IceMist,
        BloodSpurt, SwordSwing,
        RainSplash, DustCloud
    };
    
    static ParticleSystem* getSystemById(const std::string& id) {
        for (auto& system : AllSystems) {
            if (system.id == id) return &system;
        }
        return nullptr;
    }
}

// MARK: - Particle System (ECS System)

class ParticleSystemManager : public System {
private:
    std::unordered_map<std::string, ParticleSystem> particleSystems;
    std::unordered_map<std::string, std::vector<Particle>> activeParticles;  // systemId -> particles
    
public:
    ParticleSystemManager() {
        // Load predefined systems
        for (auto& system : ParticleSystems::AllSystems) {
            particleSystems[system.id] = system;
        }
    }
    
    void update(EntityManager& entities, float deltaTime) override {
        auto emitters = entities.componentStorage.getAllComponents<ParticleEmitterComponent>();
        
        for (auto& pair : emitters) {
            Entity entity = pair.first;
            auto emitter = pair.second;
            
            if (!emitter->isEmitting) continue;
            
            auto transform = entities.componentStorage.getComponent<TransformComponent>(entity);
            if (transform) {
                emitter->spawnPosition = transform->position;
            }
            
            // Spawn new particles
            while (emitter->shouldSpawn(deltaTime)) {
                spawnParticle(*emitter);
            }
            
            // Update existing particles
            updateParticles(emitter->particleSystemId, deltaTime);
            
            // Update back in storage
            entities.componentStorage.addComponent<ParticleEmitterComponent>(entity, emitter);
        }
    }
    
    void spawnParticle(const ParticleEmitterComponent& emitter) {
        Particle particle;
        
        // Random position within spawn area
        float rx = ((rand() % 100) / 100.0f - 0.5f) * emitter.spawnArea.x;
        float ry = ((rand() % 100) / 100.0f - 0.5f) * emitter.spawnArea.y;
        float rz = ((rand() % 100) / 100.0f - 0.5f) * emitter.spawnArea.z;
        
        particle.position = emitter.spawnPosition + Vec3Make(rx, ry, rz);
        
        // Random velocity
        float speedVar = emitter.speed * emitter.speedVariation;
        float actualSpeed = emitter.speed + ((rand() % 100) / 100.0f - 0.5f) * speedVar;
        
        particle.velocity = Vec3Make(
            ((rand() % 100) / 100.0f - 0.5f) * actualSpeed,
            ((rand() % 100) / 100.0f) * actualSpeed * 0.5f,
            ((rand() % 100) / 100.0f - 0.5f) * actualSpeed
        );
        
        particle.color = emitter.startColor;
        particle.size = emitter.startSize;
        particle.lifetime = emitter.lifetime;
        particle.age = 0.0f;
        
        // Add to active particles
        activeParticles[emitter.particleSystemId].push_back(particle);
        
        // Limit particle count
        auto& particles = activeParticles[emitter.particleSystemId];
        if (static_cast<int>(particles.size()) > emitter.maxParticles) {
            particles.erase(particles.begin());  // Remove oldest
        }
    }
    
    void updateParticles(const std::string& systemId, float deltaTime) {
        auto it = activeParticles.find(systemId);
        if (it == activeParticles.end()) return;
        
        auto& particles = it->second;
        
        // Update particles in reverse order (so we can remove safely)
        for (int i = static_cast<int>(particles.size()) - 1; i >= 0; i--) {
            Particle& p = particles[i];
            
            // Update age
            p.age += deltaTime;
            
            if (p.age >= p.lifetime) {
                // Remove dead particle
                particles.erase(particles.begin() + i);
                continue;
            }
            
            // Update position
            p.position += p.velocity * deltaTime;
            
            // Apply gravity
            auto system = particleSystems[systemId];
            p.velocity.y += system.gravity * deltaTime;
            
            // Interpolate color/size
            float t = p.age / p.lifetime;
            p.color = Vec4Lerp(p.color, Vec4(1, 1, 1, 0), t);  // Would use proper start/end color
            float size = p.size + (0.1f - p.size) * t;  // Would use start/end size
        }
    }
    
    std::vector<Particle> getParticlesForRendering(const std::string& systemId) {
        auto it = activeParticles.find(systemId);
        if (it != activeParticles.end()) {
            return it->second;
        }
        return {};
    }
    
    void clearAllParticles() {
        activeParticles.clear();
    }
};

} // namespace Doomnite
