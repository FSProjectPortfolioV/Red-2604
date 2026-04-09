#include "Gameplay.h"
#include "../GameComponents.h";
#include "../../DRAW/DrawComponents.h"

void Gameplay::Shatter(entt::registry& registry, const EnemyConfig& cfg)
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
		bool hasShatter = registry.all_of<Shatters>(entity);

		if (hasShatter)
		{
			auto& shatter = registry.get<Shatters>(entity);
			auto& transform = view.get<Transform>(entity);

			if (shatter.remaining > 0)
			{
				for (int i = 0; i < shatter.clones; i++)
				{
					GAME::Transform newT = transform;

					GW::MATH::GVECTORF scale = { shatter.scaleDown, shatter.scaleDown, shatter.scaleDown, 0 };
					GW::MATH::GMatrix::ScaleLocalF(newT.matrix, scale, newT.matrix);

					// Spawn child with inherited shatter values
					SpawnEnemyShatterChild(
						registry,
						manager,
						newT,
						shatter.remaining - 1,   // remaining decreases
						shatter.clones,          // same number of clones
						shatter.scaleDown,       // same scale factor
						cfg.speed,               // same speed
						cfg.modelName            // same model
					);
				}
			}

		}

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
