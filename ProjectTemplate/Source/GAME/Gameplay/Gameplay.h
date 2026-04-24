#pragma once
#include "../../DRAW/CloneEntity.h"
#include "../../GAME/Gameplay/ScoreSystem/ScoreSystem.h"
#include "../GameComponents.h"
#include "../../GAME/Gameplay/PowerUps/PowerUps.h"

struct Gameplay
{
	static void EnemyDeath(entt::registry& registry, const GAME::EnemyConfig& cfg, GAME::DamageType damageSource); //MAIN WAY OF HANDLING ENEMIES DYING

    static void GameOver(entt::registry& registry, entt::entity manager);

	static void PlayerTimers(entt::registry& registry, float dt);

	static void EnemyTimers(entt::registry& registry, float dt);

	static void EnemySpawn(entt::registry& registry, float dt);

	//MAIN WAY OF SPAWNING ENEMIES
	static void EnemySpawn(entt::registry& registry, const GAME::EnemyToken& token);

	//MAIN WAY OF MODDIFYING ENEMY MOVEMENT MID GAMEPLAY, ONLY FOR CERTAIN ENEMIES
	void Gameplay::EnemyMovementUpdates(entt::registry& registry); 

	//static void Pause(entt::registry& registry, entt::entity manager);
};