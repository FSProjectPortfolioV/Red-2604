#pragma once
#include "../../../../entt-3.13.1/single_include/entt/entt.hpp"

namespace GAME
{
	void KillPlayer(entt::registry& registry, entt::entity player, entt::entity gameManager);
	void RespawnPlayer(entt::registry& registry, float deltaTime);
	void UpdateHighscoreEntry(entt::registry& registry);
}