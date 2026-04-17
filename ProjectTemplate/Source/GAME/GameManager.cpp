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
		//Spawn enemy after a delay (for testing)!
		static bool testonce = true;
		if (registry.ctx().contains<UTIL::DeltaTime>() && testonce == true) {
			auto& manager = registry.ctx().get<DRAW::ModelManager>();
			double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;
			static float time = 0;
			time += dt;
			GAME::Transform SpawnPoint;
			GW::MATH::GMatrix::IdentityF(SpawnPoint.matrix);
			GW::MATH::GVECTORF Location = { 0,0,0,0 };
			GW::MATH::GMatrix::TranslateGlobalF(SpawnPoint.matrix, Location, SpawnPoint.matrix);
			std::string enemy = "Enemy1";
			EnemyConfig TEST = EnemyCFGCreator(registry, enemy, GAME::FormationStyle::WaveLeft);
			if (time > 3 && testonce == true) {
				SpawnFormation(registry, GAME::FormationStyle::WaveLeft, 3, 5, 1, SpawnPoint, TEST, manager, 0.5f);
				testonce = false;
			}
		}

		Gameplay::PlayerTimers(registry, dt);
		Rendering::SyncTransforms(registry);

//		Gameplay::GameOver(registry, self); --- Keep this disabled until we have our new win condition set up.
		Cleanup::Destroy(registry);
	}

	CONNECT_COMPONENT_LOGIC()
	{
		registry.on_update<GameManager>().connect<Update_GameManager>();
	}
}