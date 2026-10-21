#pragma once

#include <cstdint>
#include <vector>

#include "entities/Enemy.h"
#include "entities/Guardian.h"
#include "systems/GuardianManager.h"

class Battlefield;

struct EnemyProjectile {
    std::uint64_t ownerId;
    int laneId;
    ProjectileLevel level;
    float x;
    float y;
    float speed;
    int damage;
};

struct EnemyDefeat {
    std::uint64_t id;
    EnemyType type;
    int laneId;
    float x;
};

// Owns active enemies and their attacks. The defeat stream is for later coins.
class EnemyManager final : public EnemyTargets {
public:
    explicit EnemyManager(const Battlefield& battlefield);

    std::uint64_t spawn(EnemyType type, int laneId);
    void update(float deltaTime, std::vector<Guardian>& guardians);
    void clear();
    [[nodiscard]] std::vector<EnemyTarget> targets() const override;
    void applyDamage(std::uint64_t id, int damage) override;
    [[nodiscard]] std::vector<EnemyDefeat> takeDefeats();
    [[nodiscard]] int livingCount() const;
    [[nodiscard]] bool breachedHome() const { return breachedHome_; }
    [[nodiscard]] const std::vector<Enemy>& enemies() const { return enemies_; }
    [[nodiscard]] std::vector<Enemy>& enemies() { return enemies_; }
    [[nodiscard]] const std::vector<EnemyProjectile>& projectiles() const {
        return projectiles_;
    }

private:
    Guardian* contactFor(const Enemy& enemy, float nextX,
                         std::vector<Guardian>& guardians) const;
    void updateEnemy(Enemy& enemy, float dt, std::vector<Guardian>& guardians);
    void updateProjectiles(float dt, std::vector<Guardian>& guardians);

    const Battlefield& battlefield_;
    std::vector<Enemy> enemies_;
    std::vector<EnemyProjectile> projectiles_;
    std::vector<EnemyDefeat> defeats_;
    std::uint64_t nextId_ = 1;
    bool breachedHome_ = false;
};
