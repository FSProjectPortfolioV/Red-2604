#include "Gameplay.h"
#include "../GameComponents.h"
#include "../../DRAW/DrawComponents.h"
#include "../Rendering/Rendering.h"

void Gameplay::EnemyDeath(entt::registry& registry, const GAME::EnemyConfig& cfg, GAME::DamageType damageSource)
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
		std::cout << "Current Score: " << registry.ctx().get<ScoreSystem>().GetScore() << std::endl;
		// Dead enemy
		// Destroy the original enemy

		//PowerUp Drop
		bool isPUCarrier = registry.any_of<GAME::PUCarrier>(entity);
		if (isPUCarrier && damageSource == GAME::DamageType::PlayerBullet)
		{
			auto& spawnLoc = view.get<Transform>(entity);
			SpawnPowerUp(registry, spawnLoc.matrix);
		}

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
//update enemy timers for shooting enemies!
void Gameplay::EnemyTimers(entt::registry& registry, float dt)
{
	auto enemyView = registry.view<GAME::Enemy>();
	for (auto& e : enemyView) {
		registry.patch<GAME::Enemy>(e);
	}

}

void Gameplay::EnemySpawn(entt::registry& registry, float dt)
{
	auto& manager = registry.ctx().get<DRAW::ModelManager>();

	auto lmView = registry.view<GAME::LevelManager>();
	for (auto entity : lmView)
	{
		auto& lm = registry.get<GAME::LevelManager>(entity);
		SpawnEnemies(registry, manager, lm.level.spawnQueue);
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
			lm.level.spawnQueue
		);

		SpawnEnemies(registry, manager, lm.level.spawnQueue);
	}
}

//void Gameplay::Pause(entt::registry& registry, entt::entity manager)
//{
//	auto PauseView = registry.view<GAME::Paused>();
//	if (!PauseView.empty())
//	{
//		registry.emplace<GAME::GameOver>(manager);
//		std::cout << "Game paused\n";
//	}
//	else
//	{
//		registry.remove<GAME::GameOver>(manager);
//		std::cout << "Game unpaused\n";
//		for (auto p : PauseView)
//			registry.remove<GAME::Paused>(p);
//	}
//}

void Gameplay::EnemyMovementUpdates(entt::registry& registry)
{
 
	auto& EnView = registry.view<GAME::Enemy>();
	for (auto& entity : EnView) {
		auto& Ecfg = registry.get<GAME::EnemyConfig>(entity);
		if (!(Ecfg.Movement == GAME::FormationStyle::BigGuy) || !(Ecfg.Movement == GAME::FormationStyle::TheFinal)) {
			return;
		}
		else {
			auto& Ev = registry.get<GAME::Velocity>(entity);
			auto& Etrans = registry.get<GAME::Transform>(entity);
			if (Ecfg.Movement == GAME::FormationStyle::BigGuy) { 
				if (Etrans.matrix.row4.x > 25 && Etrans.matrix.row4.x < 26 ) {
					Ev.direction = { -Ev.direction.x ,Ev.direction.w ,Ev.direction.z ,Ev.direction.w };
				}
				else if (Etrans.matrix.row4.x < -25 && Etrans.matrix.row4.x > -26) {

					Ev.direction = { -Ev.direction.x ,Ev.direction.w ,Ev.direction.z ,Ev.direction.w };
				}
			}
			else if (Ecfg.Movement == GAME::FormationStyle::TheFinal) {

				if (Etrans.matrix.row4.x > 25 && Etrans.matrix.row4.x < 26) {
					Ev.direction = { -Ev.direction.x ,Ev.direction.w ,Ev.direction.z ,Ev.direction.w };
				}
				else if (Etrans.matrix.row4.x < -25 && Etrans.matrix.row4.x > -26) {
					Ev.direction = { -Ev.direction.x ,Ev.direction.w ,Ev.direction.z ,Ev.direction.w };
				}
			}
		}
	}
}
//checks if any enemies are supoosed to shoot
void Gameplay::EnemyFiringUpdates(entt::registry& registry)
{	
	auto enemyView = registry.view<GAME::ShootingEnemy>();
	double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	for (auto& e : enemyView) {
		auto& Ecfg = registry.get<GAME::EnemyConfig>(e);
		Ecfg.time += dt;
		if (Ecfg.time > Ecfg.fireRate) {
			registry.emplace<GAME::Firing>(e);
			Ecfg.time = 0;
		}
	}

	auto& EnBView = registry.view<GAME::Firing>();
	for (auto& entity : EnBView) {
		if (registry.any_of<GAME::Player>(entity)) {
			return;
		}
		auto& Ecfg = registry.get<GAME::EnemyConfig>(entity);

		if ((Ecfg.Movement != GAME::FormationStyle::BigGuy) && Ecfg.Movement != GAME::FormationStyle::TheFinal) {
			EnemyShoot(registry, entity, 1);
		}
		else {
			if (Ecfg.Movement == GAME::FormationStyle::BigGuy) {
				EnemyShoot(registry, entity, 2);
			}
			else if (Ecfg.Movement == GAME::FormationStyle::TheFinal) {
				EnemyShoot(registry, entity, 3);
			}
		}
		registry.remove<GAME::Firing>(entity);
	}
}
//handles the enemy shooting
void Gameplay::EnemyShoot(entt::registry& registry, entt::entity TheEnemy, int bulletcount)
{
	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
	auto& ETrans = registry.get<GAME::Transform>(TheEnemy);

	float BulletSpeed = (*config).at("EnemyBullet").at("speed").as<float>();
	if (bulletcount == 1) {
		entt::entity enemyBullet1 = registry.create(); 
													//^
													// \
													  |
		GW::MATH::GVECTORF T1 = { 0,0,1.0f,1 }; //bullet angle! (it goes UP because it gets roatated 180 later!)
		enemyBullet1 = EnemyBulletCreator(registry, ETrans,T1 , BulletSpeed);
	}
	else if (bulletcount == 2) {
		GW::MATH::GVECTORF T1 = { 0.5,0,0.5f,1 }; //bullet angle!
		GW::MATH::GVECTORF T2 = { -0.5,0,0.5f,1 }; //bullet angle!
		entt::entity enemyBullet1 = registry.create();
		entt::entity enemyBullet2 = registry.create();
		enemyBullet1 = EnemyBulletCreator(registry, ETrans, T1, BulletSpeed);
		enemyBullet2 = EnemyBulletCreator(registry, ETrans, T2, BulletSpeed);
	}
	else if (bulletcount == 3) {
		GW::MATH::GVECTORF T1 = { 0,0,1.0f,1 }; //bullet angle!
		GW::MATH::GVECTORF T2 = { 0.5,0,0.5f,1 }; //bullet angle!
		GW::MATH::GVECTORF T3 = { -0.5,0,0.5f,1 }; //bullet angle!
		entt::entity enemyBullet1 = registry.create();
		entt::entity enemyBullet2 = registry.create();
		entt::entity enemyBullet3 = registry.create();
		enemyBullet1 = EnemyBulletCreator(registry, ETrans, T1, BulletSpeed);
		enemyBullet2 = EnemyBulletCreator(registry, ETrans, T2, BulletSpeed);
		enemyBullet3 = EnemyBulletCreator(registry, ETrans, T3, BulletSpeed);
	}
}

//Makes bullets
entt::entity Gameplay::EnemyBulletCreator(entt::registry& registry, GAME::Transform StartLocation,GW::MATH::GVECTORF ShotAngle, float bulletspeed ) {
	// Create entity
	entt::entity enemyBullet = registry.create();
	auto& manager = registry.ctx().get<DRAW::ModelManager>();
	// Add components
	registry.emplace<GAME::EnemyBullets>(enemyBullet);
	// Velocity
	auto& vel = registry.emplace<GAME::Velocity>(enemyBullet);

	//set the bullet to have the correct angle
	vel.direction = ShotAngle;
	//setting up the correct rotation 
	int rotation = 180;
	rotation *= vel.direction.x;
	//give the correct speed
	if (vel.direction.x == 0) {
		vel.direction.z *= bulletspeed;
		rotation = 180;
	}
	else if (vel.direction.z == 0) {
		vel.direction.x *= bulletspeed;
		rotation = 180;
	}
	else {
		vel.direction.x *= bulletspeed / sqrt(2);
		vel.direction.z *= bulletspeed / sqrt(2);
	}
	// MeshCollection + Transform


	auto& collection = registry.emplace<DRAW::MeshCollection>(enemyBullet);
	auto& enemyBulletTransform = registry.emplace<GAME::Transform>(enemyBullet);

	// Clone model
	CloneModelToEntity(
		registry,
		manager.collections["YellowBullet"],
		collection,
		enemyBulletTransform
	);

	// Override transform with the one passed in
	enemyBulletTransform = StartLocation;
	GW::MATH::GMatrix::RotateYLocalF(enemyBulletTransform.matrix,G2D_DEGREE_TO_RADIAN_F(rotation), enemyBulletTransform.matrix);

	for (auto mesh : collection.meshEntities)
	{
		auto& inst = registry.get<DRAW::GPUInstance>(mesh);
		inst.transform = enemyBulletTransform.matrix;
	}

	// Add gameplay components
	// Collidable tag
	registry.emplace<GAME::Collidable>(enemyBullet);
	return enemyBullet;

}

