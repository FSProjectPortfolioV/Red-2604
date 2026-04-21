#pragma once
#include "../../DRAW/CloneEntity.h"

struct Gameplay
{
	static void EnemyDeath(entt::registry& registry, const GAME::EnemyConfig& cfg);

    static void GameOver(entt::registry& registry, entt::entity manager);

	static void PlayerTimers(entt::registry& registry, float dt);

	static void EnemyTimers(entt::registry& registry, float dt);

	static void EnemySpawn(entt::registry& registry, float dt);
};