// SquadSystem.hpp - C++ Squad Tactics for Doomnite"
// Handles squad formation, AI coordination, and group behaviors."

#pragma once"

#include <string>"
#include <vector>"
#include <memory>"
#include <unordered_map>"
#include "ECS.hpp"

namespace Doomnite {"

// MARK: - Squad Role"

enum class SquadRole {"
    Leader,"
    Flanker,"
    Tank,"
    Sniper,"
    Support"
};

// MARK: - Squad Member"

struct SquadMember {"
    Entity entity;"
    SquadRole role;"
    Vec3 formationOffset;"  // Offset from squad center"
    bool isAlive;"
    
    SquadMember(const Entity& memberEntity = Entity(),"
               SquadRole squadRole = SquadRole::Leader)"
        : entity(memberEntity), role(squadRole),"
          formationOffset(Vec3Zero), isAlive(true) {}
};

// MARK: - Squad Component (ECS)"

struct SquadComponent : public Component {"
    std::string squadId;"
    Entity leader;"
    std::vector<SquadMember> members;"
    Vec3 squadPosition;"
    float formationSpacing;"
    bool isAggressive;"
    float lastOrderTime;"
    
    SquadComponent(const std::string& sId = ", float spacing = 3.0f)"
        : squadId(sId), formationSpacing(spacing),"
          isAggressive(true), lastOrderTime(0.0f) {}
    
    void addMember(const Entity& member, SquadRole role = SquadRole::Leader) {"
        if (members.empty()) {"
            leader = member;"
        }"
        members.push_back(SquadMember(member, role));"
    }
    
    void removeMember(const Entity& member) {"
        members.erase(std::remove_if(members.begin(), members.end(),"
            [&member](const SquadMember& m) { return m.entity.id == member.id; }),"
            members.end());"
        if (leader.id == member.id && !members.empty()) {"
            leader = members[0].entity;  // Assign new leader"
        }"
    }
    
    Vec3 getFormationPosition(int memberIndex) const {"
        if (memberIndex < 0 || memberIndex >= static_cast<int>(members.size())) {"
            return squadPosition;"
        }"
        
        const SquadMember& member = members[memberIndex];"
        return squadPosition + member.formationOffset;"
    }
    
    void updateSquadPosition(const Vec3& newPosition) {"
        squadPosition = newPosition;"
        "
        // Update formation offsets based on leader direction"
        // Would calculate based on squad facing"
    }
    
    int getAliveCount() const {"
        int count = 0;"
        for (const auto& member : members) {"
            if (member.isAlive) count++;"
        }"
        return count;"
    }"
    
    bool isSquadDead() const {"
        return getAliveCount() == 0;"
    }"
};

// MARK: - Squad Order"

struct SquadOrder {"
    enum class OrderType {"
        Move,"
        Attack,"
        Defend,"
        Scatter,"
        Regroup"
    };"
    
    OrderType type;"
    Vec3 targetPosition;"
    Entity targetEntity;"
    float issuedTime;"
    
    SquadOrder(OrderType orderType = OrderType::Move,"
              const Vec3& target = Vec3Zero, const Entity& targetE = Entity())"
        : type(orderType), targetPosition(target),"
          targetEntity(targetE), issuedTime(0.0f) {}
};

// MARK: - Predefined Squads"

namespace Squads {"
    static std::string FireTeam = "squad_fire_team";"
    static std::string DragonSquad = "squad_dragon_riders";"
    static std::string UndeadHorde = "squad_undead_horde";"
    static std::string EliteGuard = "squad_elite_guard";"
    
    // Squad templates"
    static std::vector<SquadMember> getFireTeamTemplate() {"
        return {"
            SquadMember(Entity(), SquadRole::Leader),"
            SquadMember(Entity(), SquadRole::Flanker),"
            SquadMember(Entity(), SquadRole::Tank),"
            SquadMember(Entity(), SquadRole::Support)"
        };"
    }"
}

// MARK: - Squad System (ECS System)"

class SquadSystem : public System {"
private:"
    std::unordered_map<std::string, std::shared_ptr<SquadComponent>> squads;"
    
public:"
    void update(EntityManager& entities, float deltaTime) override {"
        auto squadComponents = entities.componentStorage.getAllComponents<SquadComponent>();"
        "
        for (auto& pair : squadComponents) {"
            Entity entity = pair.first;"
            auto squad = pair.second;"
            "
            // Update squad position to average of alive members"
            if (!squad->members.empty()) {"
                Vec3 sum = Vec3Zero;"
                int count = 0;"
                for (const auto& member : squad->members) {"
                    if (!member.isAlive) continue;"
                    auto transform = entities.componentStorage.getComponent<TransformComponent>(member.entity);"
                    if (transform) {"
                        sum += transform->position;"
                        count++;"
                    }"
                }"
                if (count > 0) {"
                    squad->squadPosition = sum / static_cast<float>(count);"
                }"
            }"
            "
            // Process squad orders"
            // Would implement order processing here"
            "
            // Update back in storage"
            entities.componentStorage.addComponent<SquadComponent>(entity, squad);"
        }"
    }"
    
    std::shared_ptr<SquadComponent> createSquad(const std::string& squadId) {"
        auto squad = std::make_shared<SquadComponent>(squadId);"
        squads[squadId] = squad;"
        return squad;"
    }"
    
    void addToSquad(const std::string& squadId, const Entity& member, EntityManager& entities) {"
        auto it = squads.find(squadId);"
        if (it != squads.end()) {"
            it->second->addMember(member);"
            "
            // Add squad component to member"
            entities.componentStorage.addComponent<SquadComponent>(member, it->second);"
        }"
    }"
    
    void issueOrder(const std::string& squadId, const SquadOrder& order) {"
        auto it = squads.find(squadId);"
        if (it != squads.end()) {"
            // Would process order queue"
        }"
    }"
};"

} // namespace Doomnite"
