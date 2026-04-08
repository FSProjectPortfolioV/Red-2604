#pragma once
#include "../../DRAW/CloneEntity.h"

struct Gameplay
{
	static void Shatter(entt::registry& registry, const EnemyConfig& cfg);

    static void GameOver(entt::registry& registry, entt::entity manager);

	static void PlayerTimers(entt::registry& registry, float dt);
};