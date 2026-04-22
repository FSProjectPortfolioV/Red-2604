#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "../CCL.h"
#include "../DRAW/CloneEntity.h"
#include "Gameplay/Gameplay.h"
#include "Physics/Physics.h"
#include "Cleanup/Cleanup.h"
#include "Rendering/Rendering.h"
#include "../../Source/UTIL/Debug.h"

namespace GAME
{
	using namespace GW::MATH;

	void Update_GameManager(entt::registry& registry, entt::entity self)
	{


		if (registry.any_of<GameOver>(self))
			return;
		double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;

		Physics::Velocity(registry, dt);
		Physics::Collision(registry);
		//Spawn enemy by pressing e, has a delay (for testing)!
		Gameplay::EnemySpawn(registry,dt);

		Gameplay::PlayerTimers(registry, dt);
		Rendering::SyncTransforms(registry);

//		Gameplay::GameOver(registry, self); --- Keep this disabled until we have our new win condition set up.
		Cleanup::Destroy(registry);
	}

	void Update_LevelManager(entt::registry& registry, entt::entity self)
	{
		auto& lm = registry.get<GAME::LevelManager>(self);
		double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;

		if (lm.levelComplete)
			return;

		lm.time += (float)dt;

		// Spawn waves at the correct time (TODO)

		// End of level
		/*if (no waves remaining)
		{
			lm.levelComplete = true;
			std::cout << "Level Complete\n";
		}*/
	}

	CONNECT_COMPONENT_LOGIC()
	{
		registry.on_update<GameManager>().connect<Update_GameManager>();
		registry.on_update<LevelManager>().connect<Update_LevelManager>();
	}
}