#include "../../gateware-main/Gateware.h"
#ifndef GAME_COMPONENTS_H_
#define GAME_COMPONENTS_H_

namespace GAME
{
	///*** Tags ***///
	struct Player {};
	struct Enemy {};
	struct Bullet {};
	struct EnemyBullets {};
	struct Collidable {};
	struct Obstacle {};
	struct ToDestroy {};
	struct GameOver {};

	///*** Components ***///
	struct Transform
	{
		GW::MATH::GMATRIXF matrix;
	};

	struct GameManager {};

	struct Firing 
	{
		float cooldown;
	};

	struct Velocity 
	{
		GW::MATH::GVECTORF direction = { 0, 0, 0, 0 };
	};

	struct Health
	{
		int HP;
	};

	struct Shatters 
	{
		int remaining;
		int clones;
		float scaleDown;
	};

	struct Invuln 
	{
		float cooldown;
	};
}// namespace GAME
#endif // !GAME_COMPONENTS_H_