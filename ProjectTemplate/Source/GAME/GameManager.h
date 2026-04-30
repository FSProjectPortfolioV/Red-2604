#pragma once
#include "../../entt-3.13.1/single_include/entt/entt.hpp"

namespace GAME
{
	void Update_GameManager(entt::registry& registry, entt::entity self);

	void Update_LevelManager(entt::registry& registry, entt::entity self);

	CONNECT_COMPONENT_LOGIC()
	{
		registry.on_update<GameManager>().connect<Update_GameManager>();
		registry.on_update<LevelManager>().connect<Update_LevelManager>();
	}
}