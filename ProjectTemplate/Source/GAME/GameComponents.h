#pragma once
#include "../../gateware-main/Gateware.h"
#include <unordered_map>

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
	struct ShootingEnemy {};
	struct PlayerDeathExplosion {};

	///*** Components ***///

	struct GameManager {};
	
	struct Transform
	{
		GW::MATH::GMATRIXF matrix;
	};

	enum FormationStyle {
		WaveLeft = 0,
		WaveRight = 1,
		ArrowHeadDown = 2,
		ArrowHeadLeft = 3,
		ArrowHeadRight = 4,
		BigGuy = 5,
		TheFinal = 6,
	};

	struct EnemyConfig
	{
		float speed;
		int hitpoints;
		float Scale;
		std::string modelName;
		FormationStyle Movement;
		int Score;
		float fireRate;
		bool isPUCarrier = false;
		float time = 0;
	};

	struct FORMATIONS {
		FormationStyle Form;
		int UsageCost;
	};

	struct EnemyToken {
		EnemyConfig Enemy;
		float SpawnRate;
		FormationStyle Style;
		Transform SpawnLocation;
		float SpeedMult;
	};

	struct Wave
	{
		float triggerTime;
		EnemyToken token;
		int EnemyNumber;
	};

	struct LevelData
	{
		std::vector<Wave> waves;
		float duration;
		std::vector<EnemyToken> spawnQueue;
	};

	struct LevelEvent
	{
		float triggerTime;
		std::string formationName;
	};

	struct LevelManager
	{
		float time = 0.0f;
		bool levelComplete = false;
		bool readyForNextLevel = false;
		LevelData level;
		int nextWaveIndex = 0;
		int levelIndex = 0; // 0 = level 1, 1 = level 2, 2 = level 3
		int difMultiplier = 1;
		int loops = 0;
		int enemyTotal = 0; //Total Spawned
		int enemyKilled = 0;//killed by player
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
		bool isRoll = false; // if true, skip blink logic
	};

	struct SpriteAnimation
	{
		int currentFrame = 0;
		int totalFrames = 16;
		int columns = 4;
		int rows = 4;
		float frameTime = 0.05f;
		float timer = 0.0f;
	};

	struct Lifetime
	{
		float timeRemaining = 1.0f;
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
		NONE,
		SideFighterPU,
		MultiShotPU,
		ScreenWipePU,
		ExtraLifePU,
		BonusPointsPU,
		COUNT	// Always keep this as the last entry to know how many power-ups there are
	};

	struct PowerUp
	{
		PowerUpType type;
		std::string modelName;

		PowerUp(PowerUpType puType)
		{
			type = puType;

			switch (type)
			{
			case PowerUpType::SideFighterPU:
				modelName = "SideFighterPU";
				break;

			case PowerUpType::MultiShotPU:
				modelName = "MultiShotPU";
				break;

			case PowerUpType::ScreenWipePU:
				modelName = "ScreenWipePU";
				break;

			case PowerUpType::ExtraLifePU:
				modelName = "ExtraLifePU";
				break;

			case PowerUpType::BonusPointsPU:
				modelName = "BonusPointsPU";
				break;
			}
		};
	};

	struct HasSideFighters
	{
		bool leftAlive = false;
		bool rightAlive = false;
		float timer = 0.0f;
	};

	struct SideFighter 
	{
		entt::entity player;
		std::string side;
		GW::MATH::GVECTORF targetOffset;
		GW::MATH::GVECTORF currentOffset;

		bool canShoot = false;
		bool isLeaving = false;
		float lerpSpeed = 4.0f;
	};

	struct MultiShot
	{
		std::vector<GW::MATH::GVECTORF> directions = {
		GW::MATH::GVECTORF{ -1, 0, 1, 0 },
		GW::MATH::GVECTORF{ 0, 0, 1, 0 },
		GW::MATH::GVECTORF{ 1, 0, 1, 0 }
		};

		float timer = 0.0f;
	};

	struct PUCarrier 
	{
	};

	enum DamageType
	{
		PlayerBullet,
		EnemyBullet,
		Collision,
		ScreenWipe
	};

	struct Paused
	{

	};

	struct SoundStorage
	{
		std::vector<bool> soundCues;
		std::vector<GW::AUDIO::GSound> sounds;
	};

	// This is defined based on the player's visible screen space.
	// If you want to use this for operations outside the screen, you will need to define a margin.
	struct Bounds
	{
		float left, right, bottom, top;
	};

	enum class ExitSide { Left, Right, Top, Bottom };

	struct EnemyExitSide
	{
		ExitSide side;
	};

	inline ExitSide GetExitSide(GAME::FormationStyle style)
	{
		switch (style)
		{
		case GAME::FormationStyle::WaveLeft:
		case GAME::FormationStyle::ArrowHeadLeft:
			return GAME::ExitSide::Right;  // enters left, exits right

		case GAME::FormationStyle::WaveRight:
		case GAME::FormationStyle::ArrowHeadRight:
			return GAME::ExitSide::Left;   // enters right, exits left

		case GAME::FormationStyle::ArrowHeadDown:
		case GAME::FormationStyle::BigGuy:
		case GAME::FormationStyle::TheFinal:
		default:
			return GAME::ExitSide::Bottom; // enters top, exits bottom
		}
	}

	struct Roll
	{
		float duration = 1.0f;      // how long the roll lasts
		float timeRemaining = 1.0f; // countdown
		float totalDuration = 1.0f; // stored for rotation calculation
		GW::MATH::GMATRIXF startMatrix; // save pre-roll orientation
	};

	struct RollCharges
	{
		int charges = 3;            // how many times the player can roll
	};

	struct QuitRequested {};
}// namespace GAME
