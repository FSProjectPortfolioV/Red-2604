#include "../../gateware-main/Gateware.h"
#ifndef GAME_COMPONENTS_H_
#define GAME_COMPONENTS_H_

namespace GAME
{
	///*** Tags ***///
	struct Player {};
	struct Enemy {};
	struct Bullet {};
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

	struct LevelManager
	{
		float time = 0.0f; // current time in level
		float scrollSpeed = 4.0f; // how fast the level scrolls
		size_t nextWave = 0; // index of the next wave to spawn
		// TODO: A vector of waves, where each wave contains enemy spawn data (type, position, formation, etc)
		bool levelComplete = false;
	};

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

	struct BackgroundObject
	{
		float speed;
		float rotationSpeed;
	};
}// namespace GAME
#endif // !GAME_COMPONENTS_H_