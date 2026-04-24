#pragma once
#include "../../GameComponents.h"
#include "../../../DRAW/DrawComponents.h"
#include "../../../DRAW/CloneEntity.h"
#include "../../../CCL.h"

void SpawnPowerUp(entt::registry& registry, const GW::MATH::GMATRIXF& transform, GAME::PowerUpType type = GAME::PowerUpType::NONE);

void PowerUpEffect(entt::registry& registry, entt::entity player, GAME::PowerUpType type);

void SideFighterPU(entt::registry& registry, entt::entity player);
void SpawnSideFighter(entt::registry& registry, entt::entity player, std::string side);

void MultiShotPU(entt::registry& registry, entt::entity player);

void Update_SideFighter(entt::registry& registry, entt::entity self);
void OnSideFighterDeath(entt::registry& registry, entt::entity self);

GAME::PowerUpType GetRandomPowerUpType(entt::registry& registry);