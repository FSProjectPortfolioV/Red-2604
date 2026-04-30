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
#include "Gameplay/ScoreSystem/HighscoreScreenController.h"
#include "Gameplay/ScoreSystem/InitialsEntrySystem.h"
#include "GameManager.h"


using namespace GW::MATH;

void GAME::Update_GameManager(entt::registry& registry, entt::entity self)
{
	if (registry.any_of<GAME::GameOver>(self))
		return;
	double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;

	Physics::Velocity(registry, dt);
	Physics::Collision(registry);
	Physics::WorldLimit(registry);
	//Spawn enemy by pressing e, has a delay (for testing)!
	Gameplay::EnemySpawn(registry, dt);
	Gameplay::PlayerTimers(registry, dt);
	Gameplay::EnemyTimers(registry, dt);
	Rendering::SyncTransforms(registry);

	//	Gameplay::GameOver(registry, self); --- Keep this disabled until we have our new win condition set up.
	Cleanup::Destroy(registry);
}

void GAME::Update_LevelManager(entt::registry& registry, entt::entity self)
{
	auto& lm = registry.get<GAME::LevelManager>(self);
	double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;

	if (lm.levelComplete)
		return;

	lm.time += (float)dt;

	// Fire any waves whose trigger time has been reached
	while (lm.nextWaveIndex < (int)lm.level.waves.size() &&
		lm.time >= lm.level.waves[lm.nextWaveIndex].triggerTime)
	{
		std::cout << "[LevelManager] Spawning wave " << lm.nextWaveIndex
			<< " at t=" << lm.time << "\n";
		GAME::EnemyToken temp = lm.level.waves[lm.nextWaveIndex].token;
		temp.Enemy.hitpoints = temp.Enemy.hitpoints  + (lm.loops);
		temp.Enemy.Score = temp.Enemy.Score + (lm.loops * 50);
		temp.Enemy.speed = temp.Enemy.speed + (lm.loops * 0.01);
		Gameplay::EnemySpawn(registry, lm.level.waves[lm.nextWaveIndex].token, lm.level.waves[lm.nextWaveIndex].EnemyNumber);
		lm.nextWaveIndex++;
	}

	// Level complete when time is up and all waves have spawned
	if (lm.time >= lm.level.duration &&
		lm.nextWaveIndex >= (int)lm.level.waves.size())
	{
		lm.levelComplete = true;
		lm.readyForNextLevel = true;
	}
}