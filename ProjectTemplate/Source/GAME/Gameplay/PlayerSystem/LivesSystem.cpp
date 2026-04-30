#include <iostream>
#include "LivesSystem.h"
#include "../../GameComponents.h"
#include "../../../DRAW/DrawComponents.h"
#include "../../../UTIL/Utilities.h"
#include "../ScoreSystem/HighscoreScreenController.h"
#include "../ScoreSystem/InitialsEntrySystem.h"
#include "../ScoreSystem/LocalHighscoreSystem.h"
#include "../ScoreSystem/ScoreSystem.h"
#include "../PowerUps/PowerUps.h"

namespace GAME
{
	void KillPlayer(entt::registry& registry, entt::entity player, entt::entity gameManager)
	{
		if (!registry.valid(player) || !registry.valid(gameManager))
		{
			return;
		}

		// Ignore hits while invuln
		if (registry.all_of<Invuln>(player))
		{
			return;
		}

		auto& health = registry.get<Health>(player);
		auto& lives = registry.get<Lives>(player);
		auto& config = registry.ctx().get<UTIL::Config>().gameConfig;

		if (health.HP <= 0)
		{
			return;
		}

		health.HP = 0;
		lives.count--;

		// Player death explosion
		auto& playerTransform = registry.get<Transform>(player);
		auto& modelManager = registry.ctx().get<DRAW::ModelManager>();

		auto found = modelManager.collections.find("ExplosionQuad");

		if (found != modelManager.collections.end())
		{
			entt::entity explosion = registry.create();

			DRAW::MeshCollection explosionMeshes;
			explosionMeshes.parent = explosion;

			for (auto sourceMesh : found->second.meshEntities)
			{
				if (!registry.valid(sourceMesh))
				{
					continue;
				}

				entt::entity clonedMesh = registry.create();

				if (registry.all_of<DRAW::GeometryData>(sourceMesh))
				{
					registry.emplace<DRAW::GeometryData>(clonedMesh, registry.get<DRAW::GeometryData>(sourceMesh));
				}

				if (registry.all_of<DRAW::GPUInstance>(sourceMesh))
				{
					auto gpu = registry.get<DRAW::GPUInstance>(sourceMesh);
					gpu.transform = playerTransform.matrix;

					GW::MATH::GVECTORF scale = { 1.0f, 1.0f, 1.0f, 0.0f };
					GW::MATH::GMatrix::ScaleGlobalF(gpu.transform, scale, gpu.transform);

					registry.emplace<DRAW::GPUInstance>(clonedMesh, gpu);
				}

				registry.emplace_or_replace<Visible>(clonedMesh).show = true;
				explosionMeshes.meshEntities.push_back(clonedMesh);
			}

			registry.emplace<DRAW::MeshCollection>(explosion, explosionMeshes);

			auto& texture = modelManager.textures["Textures/PlayerDeath/Explosion_0.png"];
			for (auto meshEntity : explosionMeshes.meshEntities)
			{
				if (registry.all_of<DRAW::GeometryData>(meshEntity))
				{
					auto& geo = registry.get<DRAW::GeometryData>(meshEntity);
					geo.textureDescriptor = texture.descriptorSet;
				}
			}

			registry.emplace<SpriteAnimation>(explosion);
			auto& animate = registry.get<SpriteAnimation>(explosion);
			animate.currentFrame = 0;
			animate.totalFrames = config->at("Explosion").at("totalFrames").as<int>();
			animate.frameTime = config->at("Explosion").at("frameTime").as<float>();

			registry.emplace<Lifetime>(explosion).timeRemaining =
				config->at("Explosion").at("lifeTime").as<float>();

			registry.emplace<PlayerDeathExplosion>(explosion);
		}

		// Disable collision while dead
		if (registry.all_of<Collidable>(player))
		{
			registry.remove<Collidable>(player);
		}

		// Stop active fire
		if (registry.all_of<Firing>(player))
		{
			registry.remove<Firing>(player);
		}

		// Hide mesh while dead
		if (registry.all_of<DRAW::MeshCollection>(player))
		{
			auto& meshes = registry.get<DRAW::MeshCollection>(player);

			for (auto mesh : meshes.meshEntities)
			{
				registry.emplace_or_replace<Visible>(mesh).show = false;
			}
		}

		if (lives.count > 0)
		{
			float delay = config->at("Player").at("respawnDelay").as<float>();
			registry.emplace_or_replace<RespawnTimer>(player).timeRemaining = delay;

			std::cout << "Player died. Lives left: " << lives.count << "\n";
		}
		else // Final death
		{
			registry.emplace_or_replace<GameOver>(gameManager);
			ClearPowerUPs(registry, player);

			auto& scoreSystem = registry.ctx().get<ScoreSystem>();
			auto& localHighscore = registry.ctx().get<LocalHighscoreSystem>();

			localHighscore.Update(scoreSystem.GetScore());

			// Check localscore to highscore
			auto& highscore = registry.ctx().get<HighscoreScreenController>();
			if (highscore.Begin(registry))
			{
				if (highscore.IsNewHighscore())
				{
					std::cout << "New Highscore\n";
					auto& initials = registry.ctx().get<InitialsEntrySystem>();
					initials.Reset();
				}
				else
				{
					std::cout << "No new highscore\n";
				}
			}
			else
			{
				std::cout << "Leaderboard failed to load\n";
			}
		}
	}

	void RespawnPlayer(entt::registry& registry, float deltaTime)
	{
		auto view = registry.view<Player, Health, Lives, Transform>();

		auto& config = registry.ctx().get<UTIL::Config>().gameConfig;
		float invulnPeriod = config->at("Player").at("invulnPeriod").as<float>();
		int hitPoints = config->at("Player").at("hitpoints").as<int>();

		for (auto player : view)
		{
			auto& health = view.get<Health>(player);

			if (registry.all_of<RespawnTimer>(player))
			{
				auto& timer = registry.get<RespawnTimer>(player);
				timer.timeRemaining -= deltaTime;

				if (timer.timeRemaining <= 0.0f)
				{
					health.HP = hitPoints;

					registry.emplace_or_replace<Collidable>(player);
					registry.emplace_or_replace<Invuln>(player).cooldown = invulnPeriod;

					// Show player meshes
					if (registry.all_of<DRAW::MeshCollection>(player))
					{
						auto& meshes = registry.get<DRAW::MeshCollection>(player);
						for (auto mesh : meshes.meshEntities)
						{
							registry.emplace_or_replace<Visible>(mesh).show = true;
						}
					}

					registry.remove<RespawnTimer>(player);

					std::cout << "Respawned\n";
				}

				continue;
			}

			if (registry.all_of<Invuln>(player))
			{
				auto& invuln = registry.get<Invuln>(player);
				invuln.cooldown -= deltaTime;

				int blinkStep = (int)(invuln.cooldown * 10.0f);
				bool visible = (blinkStep % 2 == 0);

				if (registry.all_of<DRAW::MeshCollection>(player))
				{
					auto& meshes = registry.get<DRAW::MeshCollection>(player);
					for (auto mesh : meshes.meshEntities)
					{
						registry.emplace_or_replace<Visible>(mesh).show = visible;
					}
				}

				if (invuln.cooldown <= 0.0f)
				{
					registry.remove<Invuln>(player);
					
					if (registry.all_of<DRAW::MeshCollection>(player))
					{
						auto& meshes = registry.get<DRAW::MeshCollection>(player);
						for (auto mesh : meshes.meshEntities)
						{
							registry.emplace_or_replace<Visible>(mesh).show = true;
						}
					}
				}
			}
		}
	}

	void UpdateHighscoreEntry(entt::registry& registry)
	{
		auto& highscore = registry.ctx().get<HighscoreScreenController>();

		if (!highscore.NeedsInitialsEntry())
		{
			return;
		}

		auto& initials = registry.ctx().get<InitialsEntrySystem>();
		auto& input = registry.ctx().get<UTIL::Input>();

		static bool wDown = false;
		static bool aDown = false;
		static bool sDown = false;
		static bool dDown = false;
		static bool enterDown = false;

		float state = 0.0f;

		if (input.immediateInput.GetState(G_KEY_W, state) == GW::GReturn::SUCCESS && state > 0.0f)
		{
			if (!wDown)
			{
				initials.MoveUp();
			}
			wDown = true;
		}
		else
		{
			wDown = false;
		}

		if (input.immediateInput.GetState(G_KEY_A, state) == GW::GReturn::SUCCESS && state > 0.0f)
		{
			if (!aDown)
			{
				initials.MoveLeft();
			}
			aDown = true;
		}
		else
		{
			aDown = false;
		}

		if (input.immediateInput.GetState(G_KEY_S, state) == GW::GReturn::SUCCESS && state > 0.0f)
		{
			if (!sDown)
			{
				initials.MoveDown();
			}
			sDown = true;
		}
		else
		{
			sDown = false;
		}

		if (input.immediateInput.GetState(G_KEY_D, state) == GW::GReturn::SUCCESS && state > 0.0f)
		{
			if (!dDown)
			{
				initials.MoveRight();
			}
			dDown = true;
		}
		else
		{
			dDown = false;
		}

		if (input.immediateInput.GetState(G_KEY_ENTER, state) == GW::GReturn::SUCCESS && state > 0.0f)
		{
			if (!enterDown)
			{
				std::string entered = initials.GetInitials();

				if (highscore.SubmitInitials(registry, entered))
				{
					std::cout << "Highscore saved: " << entered << "\n";
				}
				else
				{
					std::cout << "Highscore failed to save\n";
				}
			}
			enterDown = true;
		}
		else
		{
			enterDown = false;
		}
	}
	
	void PlayerExplosion(entt::registry& registry, float deltaTime)
	{
		auto view = registry.view<PlayerDeathExplosion, SpriteAnimation, Lifetime, DRAW::MeshCollection>();

		auto& modelManager = registry.ctx().get<DRAW::ModelManager>();

		for (auto entity : view)
		{
			auto& animate = view.get<SpriteAnimation>(entity);
			auto& life = view.get<Lifetime>(entity);
			auto& meshes = view.get<DRAW::MeshCollection>(entity);

			animate.timer += deltaTime;
			life.timeRemaining -= deltaTime;

			if (animate.timer >= animate.frameTime)
			{
				animate.timer = 0.0f;
				animate.currentFrame++;
			}

			if (animate.currentFrame >= animate.totalFrames || life.timeRemaining <= 0.0f)
			{
				for (auto meshEntity : meshes.meshEntities)
				{
					if (registry.valid(meshEntity))
					{
						registry.destroy(meshEntity);
					}
				}

				registry.destroy(entity);
				continue;
			}

			// Swap mesh to the next frame
			std::string frameName = "Textures/PlayerDeath/Explosion_" + std::to_string(animate.currentFrame) + ".png";

			if (modelManager.textures.find(frameName) != modelManager.textures.end())
			{
				auto& texture = modelManager.textures[frameName];

				for (auto meshEntity : meshes.meshEntities)
				{
					if (registry.all_of<DRAW::GeometryData>(meshEntity))
					{
						auto& geo = registry.get<DRAW::GeometryData>(meshEntity);
						geo.textureDescriptor = texture.descriptorSet;
					}
				}
			}
		}
	}
}