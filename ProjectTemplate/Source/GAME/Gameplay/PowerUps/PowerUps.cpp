#include "PowerUps.h"

void SpawnPowerUp(entt::registry& registry, const GW::MATH::GMATRIXF& transform)
{
    entt::entity powerUp = registry.create();
    auto& vel = registry.emplace<GAME::Velocity>(powerUp);

    GW::MATH::GVECTORF powerUpDir = { 0, 0, -1, 0 };
    powerUpDir.z *= 5.0f;
    vel.direction = powerUpDir;

    registry.emplace<GAME::PowerUp>(powerUp);
    registry.emplace<GAME::Collidable>(powerUp);

    auto& powerUpTransform = registry.emplace<GAME::Transform>(powerUp);
    auto& manager = registry.ctx().get<DRAW::ModelManager>();
    auto& powerUpCollection = registry.emplace<DRAW::MeshCollection>(powerUp);

    CloneModelToEntity(
        registry,
        manager.collections["SideFighterPU"],
        powerUpCollection,
        powerUpTransform
    );

    // Spawn it in random position
    float spawnRange = 100.0f;
    float randX = ((float)rand() / RAND_MAX) * spawnRange - (spawnRange / 2.0f);
    float randZ = ((float)rand() / RAND_MAX) * spawnRange - (spawnRange / 2.0f);

    GW::MATH::GMatrix::TranslateLocalF(
        powerUpTransform.matrix,
        GW::MATH::GVECTORF{ randX, 0.0f, randZ, 0.0f },
        powerUpTransform.matrix
    );

    for (auto mesh : powerUpCollection.meshEntities)
    {
        auto& inst = registry.get<DRAW::GPUInstance>(mesh);
        inst.transform = powerUpTransform.matrix;
    }
}