#!/bin/bash
echo "=== Testing Compilation ==="

# Fix quotes in one file as test
sed -i 's/[\u201c\u201d]/\"/g; s/[\u2018\u2019]/'"'/g' Sources/DoomniteEngine/Core/FixedMath.h 2>/dev/null

# Try compiling the actual engine
g++ -std=c++17 -I./Sources \
    Sources/DoomniteEngine/Core/CompleteGameEngine.cpp \
    Sources/DoomniteEngine/Core/ECS.cpp \
    Sources/Game/Player/Combat/CombatSystem.cpp \
    Sources/Game/Player/Movement/MovementSystem.cpp \
    Sources/Game/Weapons/WeaponSystem.cpp \
    Sources/Game/Magic/Spells/SpellSystem.cpp \
    Sources/Game/Armor/ArmorSystem.cpp \
    Sources/Game/Companions/CompanionsSystem.cpp \
    Sources/Game/Player/Progression/PlayerProgression.cpp \
    test_main.cpp \
    -o test_engine 2>&1 | head -20

if [ -f test_engine ]; then
    echo "=== COMPILED! Running test ==="
    ./test_engine
else
    echo "=== COMPILATION FAILED ==="
fi
