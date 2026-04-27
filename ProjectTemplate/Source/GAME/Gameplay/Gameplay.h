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

	//If enemies have a firing tag. They'll shoot bullets. Will change for certain enmeies
	void Gameplay::EnemyFiringUpdates(entt::registry& registry);
	//handles enemy shooting, bullet count will only affect amount shot in one shot in a spread if there's more than one.
	void Gameplay::EnemyShoot(entt::registry& registry, entt::entity TheEnemy, int bulletcount);
	//makes bullets.
	entt::entity Gameplay::EnemyBulletCreator(entt::registry& registry, const DRAW::ModelManager& manager, GAME::Transform StartLocation, GW::MATH::GVECTORF ShotAngle, float bulletspeed);

	//static void Pause(entt::registry& registry, entt::entity manager);
};