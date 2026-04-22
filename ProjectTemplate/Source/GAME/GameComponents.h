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

	struct Lives
	{
		int count = 3;
	};

	struct RespawnTimer
	{
		float timeRemaining = 0.0f;
	};

	struct Visible
	{
		bool show = true;
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

	enum PowerUpType
	{
		None,
		SideFighterPU,
	};

	struct PowerUp
	{
		PowerUpType type;
	};

	struct HasSideFighters
	{
		bool leftAlive = false;
		bool rightAlive = false;
	};

	struct SideFighter 
	{
		entt::entity player;
		std::string side;
		GW::MATH::GVECTORF targetOffset;
		GW::MATH::GVECTORF currentOffset;

		bool canShoot = false;
		float lerpSpeed = 4.0f;
	};

	struct Paused
	{

	};

	// This is defined based on the player's visible screen space.
	// If you want to use this for operations outside the screen, you will need to define a margin.
	struct Bounds
	{
		float left, right, bottom, top;
	};;

}// namespace GAME
#endif // !GAME_COMPONENTS_H_