// DoomniteEngine.mm - ObjC++ Bridge"
// Connects C++ doomnite_final engine to iOS"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "doomnite_final.cpp"  // Include the C++ engine

// MARK: - C++ Wrapper"

extern "C" {
    void* CreateDoomniteEngine() {
        auto engine = new DoomniteEngine();
        return static_cast<void*>(engine);
    }
    
    void DestroyDoomniteEngine(void* enginePtr) {
        auto* engine = static_cast<DoomniteEngine*>(enginePtr);
        delete engine;
    }
    
    void UpdateDoomniteEngine(void* enginePtr, float dt) {
        auto* engine = static_cast<DoomniteEngine*>(enginePtr);
        if (engine) engine->update(dt);
    }
    
    void StartNewGame(void* enginePtr) {
        auto* engine = static_cast<DoomniteEngine*>(enginePtr);
        if (engine) engine->startNewGame();
    }
    
    void SaveGame(void* enginePtr) {
        auto* engine = static_cast<DoomniteEngine*>(enginePtr);
        if (engine) engine->saveGame();
    }
    
    float GetPlayerHealth(void* enginePtr) {
        auto* engine = static_cast<DoomniteEngine*>(enginePtr);
        if (engine) return engine->getPlayerHealth();
        return 0.0f;
    }
    
    bool IsPlayerAlive(void* enginePtr) {
        auto* engine = static_cast<DoomniteEngine*>(enginePtr);
        if (engine) return engine->isPlayerAlive();
        return false;
    }
}

// MARK: - ObjC++ ObjC Class"

@interface DoomniteEngine : NSObject {
    void* _enginePtr;
}

@property (nonatomic, readonly) void* enginePtr;

- (instancetype)init {
    self = [super init];
    if (self) {
        _enginePtr = CreateDoomniteEngine();
    }
    return self;
}

- (void)update:(float)deltaTime {
    if (_enginePtr) {
        UpdateDoomniteEngine(_enginePtr, deltaTime);
    }
}

- (void)startNewGame {
    if (_enginePtr) {
        StartNewGame(_enginePtr);
    }
}

- (void)saveGame {
    if (_enginePtr) {
        SaveGame(_enginePtr);
    }
}

- (float)playerHealth {
    if (_enginePtr) {
        return GetPlayerHealth(_enginePtr);
    }
    return 0.0f;
}

- (BOOL)isPlayerAlive {
    if (_enginePtr) {
        return IsPlayerAlive(_enginePtr) ? YES : NO;
    }
    return NO;
}

- (void)dealloc {
    if (_enginePtr) {
        DestroyDoomniteEngine(_enginePtr);
        _enginePtr = nullptr;
    }
    [super dealloc];
}

@end
