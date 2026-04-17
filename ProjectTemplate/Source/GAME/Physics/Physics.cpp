#include "Physics.h"
#include "../GameComponents.h"
#include "../../DRAW/CloneEntity.h"
#include "../../GAME/Gameplay/PowerUps/PowerUps.h"



void HurtPlayer(entt::registry& registry, entt::entity player)
{
	// Get invul period from file
	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;

	// If we already have invul, return
	if (registry.all_of<GAME::Invuln>(player))
		return;

	// else decrement health
	auto& health = registry.get<GAME::Health>(player);
	health.HP -= 1;

	std::cout << "Player HP: " << health.HP << "\n";

	// Add invulnerability
	auto& invul = registry.emplace<GAME::Invuln>(player);
	invul.cooldown = (*config).at("Player").at("invulnPeriod").as<float>();;
}

void Physics::Velocity(entt::registry& registry, float dt)
{
	// Velocity System
	auto velView = registry.view<GAME::Transform, GAME::Velocity>();

	for (auto entity : velView)
	{
		auto& transform = velView.get<GAME::Transform>(entity);
		auto& vel = velView.get<GAME::Velocity>(entity);

		// Move entity by velocity * dt
		GW::MATH::GMATRIXF newMat;
		GW::MATH::GMatrix::TranslateLocalF(
			transform.matrix,
			GW::MATH::GVECTORF{
				vel.direction.x * (float)dt,
				vel.direction.y * (float)dt,
				vel.direction.z * (float)dt,
				0
			},
			newMat
		);

		transform.matrix = newMat;
		float currentSpeed;
		GW::MATH::GVector::MagnitudeF(vel.direction, currentSpeed);
	}
}

void Physics::Collision(entt::registry& registry)
{
	using namespace GAME;
	using namespace GW::MATH;

	// Collision System
	auto collisions = registry.view<Transform, DRAW::MeshCollection, Collidable>();

	for (auto a = collisions.begin(); a != collisions.end(); ++a)
	{
		auto colA = registry.get<DRAW::MeshCollection>(*a).collider;
		auto& transA = registry.get<Transform>(*a).matrix;

		// Scale
		GVECTORF scaleA;
		GMatrix::GetScaleF(transA, scaleA);
		colA.extent.x *= scaleA.x;
		colA.extent.y *= scaleA.y;
		colA.extent.z *= scaleA.z;
		colA.center.w = 1;

		// Translate
		GMatrix::VectorXMatrixF(transA, colA.center, colA.center);

		// Rotate
		GQUATERNIONF qA;
		GQuaternion::SetByMatrixF(transA, qA);
		GQuaternion::MultiplyQuaternionF(qA, colA.rotation, colA.rotation);

		auto b = a;
		for (++b; b != collisions.end(); ++b)
		{
			auto colB = registry.get<DRAW::MeshCollection>(*b).collider;
			auto& transB = registry.get<Transform>(*b).matrix;

			// Scale
			GVECTORF scaleB;
			GMatrix::GetScaleF(transB, scaleB);
			colB.extent.x *= scaleB.x;
			colB.extent.y *= scaleB.y;
			colB.extent.z *= scaleB.z;

			// Translate
			GMatrix::VectorXMatrixF(transB, colB.center, colB.center);

			// Rotate
			GQUATERNIONF qB;
			GQuaternion::SetByMatrixF(transB, qB);
			GQuaternion::MultiplyQuaternionF(qB, colB.rotation, colB.rotation);

			GCollision::GCollisionCheck result;
			GCollision::TestOBBToOBBF(colA, colB, result);
			if (result == GCollision::GCollisionCheck::COLLISION)
			{
				//Collision Happened

				// Case: Bullet To Wall - Mark bullet for destruction
				if (registry.all_of<Bullet>(*a) && registry.all_of<Obstacle>(*b))
				{
					registry.emplace_or_replace<ToDestroy>(*a);
				}
				if (registry.all_of<Bullet>(*b) && registry.all_of<Obstacle>(*a))
				{
					registry.emplace_or_replace<ToDestroy>(*b);
				}
				 //UPDATED!
				// Case: Enemy to Wall - Enemy Gets Destoryed!
				if (registry.all_of<Enemy>(*a) && registry.all_of<Obstacle>(*b))
				{
					if (!registry.any_of<Invuln>(*a)) {
						registry.emplace_or_replace<ToDestroy>(*a);
					}
					
				}
				if (registry.all_of<Enemy>(*b) && registry.all_of<Obstacle>(*a))
				{
					if (!registry.any_of<Invuln>(*b)) {
						registry.emplace_or_replace<ToDestroy>(*b);
					}
					
				}

				// Case: Bullet to Enemy - Enemy takes damage, bullet gets destroyed
				if (registry.all_of<Enemy>(*a) && registry.all_of<Bullet>(*b))
				{
					auto& health = registry.get<Health>(*a);
					health.HP -= 1;

					registry.emplace_or_replace<ToDestroy>(*b);
				}

				if (registry.all_of<Enemy>(*b) && registry.all_of<Bullet>(*a))
				{
					auto& health = registry.get<Health>(*b);
					health.HP -= 1;

					registry.emplace_or_replace<ToDestroy>(*a);
				}

				// Case: Enemy to Player - Hurt the player
				if (registry.all_of<Enemy>(*a) && registry.all_of<Player>(*b))
				{
					HurtPlayer(registry, *b);
				}
				if (registry.all_of<Enemy>(*b) && registry.all_of<Player>(*a))
				{
					HurtPlayer(registry, *a);
				}

				// Case: Player to Power-Ups - Player gets the power-up, power-up gets destroyed
				if (registry.all_of<Player>(*a) && registry.all_of<PowerUp>(*b))
				{
					auto& powerUp = registry.get<GAME::PowerUp>(*b);

					PowerUpEffect(registry, *a, powerUp.type);

					registry.emplace_or_replace<GAME::ToDestroy>(*b);
				}
				if (registry.all_of<Player>(*b) && registry.all_of<PowerUp>(*a))
				{
					auto& powerUp = registry.get<GAME::PowerUp>(*a);

					PowerUpEffect(registry, *b, powerUp.type);

					registry.emplace_or_replace<GAME::ToDestroy>(*a);
				}
			}
		}
	}
}
