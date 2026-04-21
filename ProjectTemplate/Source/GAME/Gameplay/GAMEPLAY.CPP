#include "Gameplay.h"
#include "../GameComponents.h"
#include "../../DRAW/DrawComponents.h"
#include "../Rendering/Rendering.h"

void Gameplay::EnemyDeath(entt::registry& registry, const GAME::EnemyConfig& cfg)
{
	using namespace GAME;
	auto view = registry.view<Enemy, Health, Transform, DRAW::MeshCollection>();
	auto& manager = registry.ctx().get<DRAW::ModelManager>();

	for (auto entity : view)
	{
		auto& health = view.get<Health>(entity);

		if (health.HP > 0)
			continue; // still alive

		// Dead enemy
		// Destroy the original enemy
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
	auto invView = registry.view<GAME::Player, GAME::Invuln>();
	for (auto p : invView)
	{
		auto& inv = invView.get<GAME::Invuln>(p);
		inv.cooldown -= dt;

		if (inv.cooldown <= 0)
			registry.remove<GAME::Invuln>(p);
	}

	auto playerView = registry.view<GAME::Player>();
	for (auto p : playerView)
		registry.patch<GAME::Player>(p);
}

void Gameplay::EnemyTimers(entt::registry& registry, float dt)
{
	auto invView = registry.view<GAME::Enemy, GAME::Invuln>();
	for (auto e : invView)
	{
		auto& inv = invView.get<GAME::Invuln>(e);
		inv.cooldown -= dt;

		if (inv.cooldown <= 0)
			registry.remove<GAME::Invuln>(e);
	}

	//auto playerView = registry.view<GAME::Player>();
	//for (auto p : playerView)
	//	registry.patch<GAME::Player>(p);
}

void Gameplay::EnemySpawn(entt::registry& registry, float dt) {

	static bool testonce = true;
	float state = 0.0f;
	auto& input = registry.ctx().get<UTIL::Input>();
	static float time = 0;
	time += dt;
	static std::vector<GAME::EnemyToken> CurrentList; //MAIN ENEMY LIST
	static int OverallCost = 8;
	auto& manager = registry.ctx().get<DRAW::ModelManager>();
	if (input.immediateInput.GetState(G_KEY_E, state) == GW::GReturn::SUCCESS && state > 0.0f) {
		
		GAME::Transform SpawnPoint;
		GW::MATH::GMatrix::IdentityF(SpawnPoint.matrix);
		GW::MATH::GVECTORF Location = { 3,0,0,0 };
		GW::MATH::GMatrix::TranslateGlobalF(SpawnPoint.matrix, Location, SpawnPoint.matrix);
		std::string enemy = "Enemy1";
		GAME::EnemyConfig TEST = EnemyCFGCreator(registry, enemy, GAME::FormationStyle::WaveLeft);
		if (time > 2) {
			SpawnFormation(registry, GAME::FormationStyle::WaveLeft, 3, 5, 1, SpawnPoint, TEST, manager, 0.5f,CurrentList,1);
			time = 0;
		}
	}
	SpawnEnemies(registry,manager, CurrentList,OverallCost);
}