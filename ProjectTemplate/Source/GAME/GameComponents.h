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

	struct LevelEvent 
	{
		float triggerTime;
		std::string formationName;
	};

	struct LevelManager {
		float time = 0;
		int tokenBudget = 10;
		int tokensAvailable = 10;

		size_t nextEvent = 0;
		std::vector<LevelEvent> timeline;

		std::queue<std::string> formationQueue; // names of formations waiting to spawn
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

	struct ScrollingBackground
	{
		float scrollSpeed;
	};

	struct WindowBounds
	{
		float left, right, top, bottom;
	};
}// namespace GAME
#endif // !GAME_COMPONENTS_H_