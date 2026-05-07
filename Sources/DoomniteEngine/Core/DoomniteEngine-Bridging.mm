// DoomniteEngine-Bridging.mm - Objective-C++ Bridge
// Connects C++ engine to Swift/iOS
// Compile this as Objective-C++ (.mm) to mix C++ and Objective-C

#import <Foundation/Foundation.h>
#include "MathTypes.h"
#include "ECS.hpp"
#include "WeaponSystem.hpp"

// MARK: - Swift-callable C++ Wrappers

extern "C" {
    // Entity creation
    uint32_t CreateEntity(Doomnite::EntityManager* manager) {
        Doomnite::Entity entity = manager->createEntity();
        return entity.id;
    }
    
    void DestroyEntity(Doomnite::EntityManager* manager, uint32_t entityId) {
        Doomnite::Entity entity(entityId, 0);
        manager->destroyEntity(entity);
    }
    
    // Component management
    void AddTransformComponent(Doomnite::EntityManager* manager, uint32_t entityId,
                               float posX, float posY, float posZ,
                               float rotX, float rotY, float rotZ) {
        Doomnite::Entity entity(entityId, 0);
        auto transform = std::make_shared<Doomnite::TransformComponent>(
            Doomnite::Vec3Make(posX, posY, posZ),
            Doomnite::Vec3Make(rotX, rotY, rotZ),
            Doomnite::Vec3Make(1, 1, 1)
        );
        manager->componentStorage.addComponent<Doomnite::TransformComponent>(entity, transform);
    }
    
    void AddVelocityComponent(Doomnite::EntityManager* manager, uint32_t entityId,
                              float velX, float velY, float velZ) {
        Doomnite::Entity entity(entityId, 0);
        auto velocity = std::make_shared<Doomnite::VelocityComponent>(
            Doomnite::Vec3Make(velX, velY, velZ),
            Doomnite::Vec3Zero
        );
        manager->componentStorage.addComponent<Doomnite::VelocityComponent>(entity, velocity);
    }
    
    // Weapon creation
    void* CreateWeapon(Doomnite::WeaponType type, float damage, float fireRate) {
        Doomnite::WeaponStats stats(damage, fireRate, 100.0f, 30, 2.0f, 0.9f, 1.0f);
        auto weapon = new Doomnite::WeaponComponent(type, stats);
        return static_cast<void*>(weapon);
    }
    
    void AddWeaponToEntity(Doomnite::EntityManager* manager, uint32_t entityId, void* weaponPtr) {
        Doomnite::Entity entity(entityId, 0);
        auto weapon = static_cast<Doomnite::WeaponComponent*>(weaponPtr);
        manager->componentStorage.addComponent<Doomnite::WeaponComponent>(entity,
            std::shared_ptr<Doomnite::WeaponComponent>(weapon));
    }
    
    void SocketRuneToWeapon(void* weaponPtr, const char* runeId) {
        auto weapon = static_cast<Doomnite::WeaponComponent*>(weaponPtr);
        std::string id(runeId);
        
        // Find rune by ID (simplified - would use a lookup)
        if (id == "rune_fire") {
            auto rune = std::make_shared<Doomnite::Rune>(
                "rune_fire", "Rune of Fire", Doomnite::ElementType::Fire, 1.0f, 10.0f
            );
            weapon->socketRune(rune);
        }
    }
    
    // System updates
    void UpdateMovementSystem(Doomnite::EntityManager* manager, float deltaTime) {
        Doomnite::MovementSystem system;
        system.update(*manager, deltaTime);
    }
    
    void UpdateWeaponSystem(Doomnite::EntityManager* manager, float deltaTime) {
        Doomnite::WeaponSystem system;
        system.update(*manager, deltaTime);
    }
}

// MARK: - Swift Bridging Header

@interface DoomniteEngineBridge : NSObject

@property (nonatomic) Doomnite::EntityManager* entityManager;

- (instancetype)init;
- (void)dealloc;

// Entity management
- (uint32_t)createEntity;
- (void)destroyEntity:(uint32_t)entityId;

// Component management (Swift-callable)
- (void)addTransformToEntity:(uint32_t)entityId
                        position:(simd_float3)position
                        rotation:(simd_float3)rotation;

- (void)addVelocityToEntity:(uint32_t)entityId
                      velocity:(simd_float3)velocity;

// Weapon system
- (void*)createWeaponOfType:(NSString*)type damage:(float)damage fireRate:(float)fireRate;
- (void)addWeapon:(void*)weapon toEntity:(uint32_t)entityId;
- (void)socketRune:(NSString*)runeId toWeapon:(void*)weaponPtr;

// System updates
- (void)updateMovementSystem:(float)deltaTime;
- (void)updateWeaponSystem:(float)deltaTime;

@end

@implementation DoomniteEngineBridge

- (instancetype)init {
    self = [super init];
    if (self) {
        _entityManager = new Doomnite::EntityManager();
    }
    return self;
}

- (void)dealloc {
    delete _entityManager;
    [super dealloc]; // Only if not using ARC
}

- (uint32_t)createEntity {
    return CreateEntity(_entityManager);
}

- (void)destroyEntity:(uint32_t)entityId {
    DestroyEntity(_entityManager, entityId);
}

- (void)addTransformToEntity:(uint32_t)entityId
                        position:(simd_float3)position
                        rotation:(simd_float3)rotation {
    AddTransformComponent(_entityManager, entityId,
                          position.x, position.y, position.z,
                          rotation.x, rotation.y, rotation.z);
}

- (void)addVelocityToEntity:(uint32_t)entityId
                      velocity:(simd_float3)velocity {
    AddVelocityComponent(_entityManager, entityId,
                         velocity.x, velocity.y, velocity.z);
}

- (void*)createWeaponOfType:(NSString*)type damage:(float)damage fireRate:(float)fireRate {
    Doomnite::WeaponType weaponType = Doomnite::WeaponType::Pistol; // Default
    if ([type isEqualToString:@"staff"]) {
        weaponType = Doomnite::WeaponType::Staff;
    } else if ([type isEqualToString:@"sword"]) {
        weaponType = Doomnite::WeaponType::Sword;
    }
    return CreateWeapon(weaponType, damage, fireRate);
}

- (void)addWeapon:(void*)weapon toEntity:(uint32_t)entityId {
    AddWeaponToEntity(_entityManager, entityId, weapon);
}

- (void)socketRune:(NSString*)runeId toWeapon:(void*)weaponPtr {
    SocketRuneToWeapon(weaponPtr, [runeId UTF8String]);
}

- (void)updateMovementSystem:(float)deltaTime {
    UpdateMovementSystem(_entityManager, deltaTime);
}

- (void)updateWeaponSystem:(float)deltaTime {
    UpdateWeaponSystem(_entityManager, deltaTime);
}

@end
