#include "Gameplay.h"
#include "../GameComponents.h"
#include "../../DRAW/DrawComponents.h"
#include "../Rendering/Rendering.h"

void Gameplay::EnemyDeath(entt::registry& registry, const GAME::EnemyConfig& cfg)
{
	using namespace GAME;
	auto view = registry.view<Enemy, Health, Transform, DRAW::MeshCollection>();
	auto& manager = registry.ctx().get<DRAW::ModelManager>();

	auto lmView = registry.view<GAME::LevelManager>();
	auto lmEntity = lmView.front();
	auto& lm = registry.get<GAME::LevelManager>(lmEntity);

	for (auto entity : view)
	{
		auto& health = view.get<Health>(entity);
		if (health.HP > 0)
			continue;

		registry.ctx().get<ScoreSystem>().AddPoints(cfg.Score);
		lm.level.tokensAvailable++;
		std::cout << "Current Score: " << registry.ctx().get<ScoreSystem>().GetScore() << std::endl;

		registry.emplace_or_replace<ToDestroy>(entity);
	}
}

void Gameplay::GameOver(entt::registry& registry, entt::entity manager)
{
	// Dead Player Check System
	auto players = registry.view<GAME::Player, GAME::Health>();

	bool allDead = true;

	for (auto p : players)
	{
		if (players.get<GAME::Health>(p).HP > 0)
		{
			allDead = false;
			break;
		}
	}

	if (allDead)
	{
		registry.emplace<GAME::GameOver>(manager);
		std::cout << "You lose, game over\n";
	}

	// Dead enemy check system
	auto enemies = registry.view<GAME::Enemy>();

	if (enemies.empty())
	{
		registry.emplace<GAME::GameOver>(manager);
		std::cout << "You win, good job!\n";
	}
}

void Gameplay::PlayerTimers(entt::registry& registry, float dt)
{
	auto playerView = registry.view<GAME::Player>();
	for (auto p : playerView)
		registry.patch<GAME::Player>(p);
}

void Gameplay::EnemyTimers(entt::registry& registry, float dt)
{

}

void Gameplay::EnemySpawn(entt::registry& registry, float dt)
{
	auto& manager = registry.ctx().get<DRAW::ModelManager>();

	auto lmView = registry.view<GAME::LevelManager>();
	for (auto entity : lmView)
	{
		auto& lm = registry.get<GAME::LevelManager>(entity);
		SpawnEnemies(registry, manager, lm.level.spawnQueue, lm.level.tokensAvailable);
	}
}

void Gameplay::EnemySpawn(entt::registry& registry, const GAME::EnemyToken& token)
{
	auto& manager = registry.ctx().get<DRAW::ModelManager>();

	auto lmView = registry.view<GAME::LevelManager>();
	for (auto entity : lmView)
	{
		auto& lm = registry.get<GAME::LevelManager>(entity);

		auto& bounds = registry.ctx().get<GAME::Bounds>();
		GAME::Transform spawnTransform = GetOffscreenSpawn(
			bounds,
			token.Style,
			token.SpawnLocation.matrix.row4.x,
			token.SpawnLocation.matrix.row4.z,
			10.0f
		);

		SpawnFormation(
			registry,
			token.Style,
			5, 5, 1,
			spawnTransform,
			token.Enemy,
			manager,
			token.SpeedMult,
			lm.level.spawnQueue,
			token.UsageCost
		);

		SpawnEnemies(registry, manager, lm.level.spawnQueue, lm.level.tokensAvailable);
	}
}

void Gameplay::Pause(entt::registry& registry, entt::entity manager)
{
	auto PauseView = registry.view<GAME::Paused>();
	if (PauseView.empty())
	{
		registry.emplace<GAME::GameOver>(manager);
		std::cout << "Game paused\n";
	}
	else
	{
		registry.remove<GAME::GameOver>(manager);
		std::cout << "Game unpaused\n";
		for (auto p : PauseView)
			registry.remove<GAME::Paused>(p);
	}
}

void Gameplay::EnemyMovementUpdates(entt::registry& registry)
{
 //WIP
}
