#include "PowerUps.h"
#include "../../../CCL.h"
#include "../Gameplay.h"


void SpawnPowerUp(entt::registry& registry, const GW::MATH::GMATRIXF& transform, GAME::PowerUpType type)
{
    entt::entity powerUp = registry.create();
    auto& vel = registry.emplace<GAME::Velocity>(powerUp);

    GW::MATH::GVECTORF powerUpDir = { 0, 0, -1, 0 };
    powerUpDir.z *= 5.0f;
    vel.direction = powerUpDir;


    GAME::PowerUp* powerUpComponent = nullptr;
    if (type == GAME::PowerUpType::NONE) 
    {
		//Randomly select a power-up type if NONE is specified
        powerUpComponent = &registry.emplace<GAME::PowerUp>(powerUp, GetRandomPowerUpType(registry));

    }
    else
    {
        powerUpComponent = &registry.emplace<GAME::PowerUp>(powerUp, type);
    }
    
    registry.emplace<GAME::Collidable>(powerUp);

    auto& powerUpTransform = registry.emplace<GAME::Transform>(powerUp);
    auto& manager = registry.ctx().get<DRAW::ModelManager>();
    auto& powerUpCollection = registry.emplace<DRAW::MeshCollection>(powerUp);

    CloneModelToEntity(
        registry,
        manager.collections[powerUpComponent->modelName],
        powerUpCollection,
        powerUpTransform
    );

    GW::MATH::GMatrix::TranslateGlobalF(
        powerUpTransform.matrix,
        GW::MATH::GVECTORF{ transform.row4.x, 0.0f, transform.row4.z, 0.0f },
        powerUpTransform.matrix
    );
}

void PowerUpEffect(entt::registry& registry, entt::entity player, GAME::PowerUpType type)
{
    switch (type)
    {
        case GAME::PowerUpType::SideFighterPU:

		    SideFighterPU(registry, player);
            break;

	    case GAME::PowerUpType::MultiShotPU:
            MultiShotPU(registry, player);
            break;

            case GAME::PowerUpType::ScreenWipePU:
                ScreenWipePU(registry);
				break;
        default:
            break;
	}
}

void SideFighterPU(entt::registry& registry, entt::entity player)
{
    if (!registry.any_of<GAME::HasSideFighters>(player))
    {
        registry.emplace<GAME::HasSideFighters>(player, GAME::HasSideFighters{ true, true });

        SpawnSideFighter(registry, player, "LEFT");
        SpawnSideFighter(registry, player, "RIGHT");
    }
    else
    {
        auto& sideFighterData = registry.get<GAME::HasSideFighters>(player);

        if (!sideFighterData.leftAlive || !sideFighterData.rightAlive)
        {
            if (!sideFighterData.leftAlive)
            {
                SpawnSideFighter(registry, player, "LEFT");
                sideFighterData.leftAlive = true;
            }
            if (!sideFighterData.rightAlive)
            {
				SpawnSideFighter(registry, player, "RIGHT");
                sideFighterData.rightAlive = true;
            }
        }
        else
        {
            //Player already has both! Give them bonus points instead?.
        }
    }
}

void SpawnSideFighter(entt::registry& registry, entt::entity player, std::string side)
{
    entt::entity sideFighter = registry.create();

    GW::MATH::GVECTORF targetOffset = { 0, 0, 0, 0 };
    if (side == "LEFT")
    {
        targetOffset = { -4.5f, 0.0f, -2.0f, 0.0f }; // X is negative (left)
    }
    else if (side == "RIGHT")
    {
        targetOffset = { 4.5f, 0.0f, -2.0f, 0.0f };  // X is positive (right)
    }

    GW::MATH::GVECTORF currentOffset = targetOffset;
    currentOffset.z = -50.0f;

    registry.emplace<GAME::SideFighter>(sideFighter, player, side, targetOffset, currentOffset);
    registry.emplace<GAME::Collidable>(sideFighter);
    auto& sideFighterTrans = registry.emplace<GAME::Transform>(sideFighter);

    //Temporary, model will change
    auto& manager = registry.ctx().get<DRAW::ModelManager>();
    auto config = registry.ctx().get<UTIL::Config>().gameConfig;
    std::string playerModelName = config->at("Player").at("model").as<std::string>();

    auto& wingmanCollection = registry.emplace<DRAW::MeshCollection>(sideFighter);

    CloneModelToEntity(
        registry,
        manager.collections[playerModelName],
        wingmanCollection,
        sideFighterTrans
    );

    //for (auto mesh : wingmanCollection.meshEntities)
    //{
    //    auto& inst = registry.get<DRAW::GPUInstance>(mesh);
    //    inst.transform = sideFighterTrans.matrix;
    //}
}

void MultiShotPU(entt::registry& registry, entt::entity player)
{
	registry.emplace_or_replace<GAME::MultiShot>(player);
}
void ScreenWipePU(entt::registry& registry)
{
	auto& allEnemies = registry.view<GAME::Enemy>();

    for (auto enemy : allEnemies)
    {
		auto& enemyTrans = registry.get<GAME::Transform>(enemy);

        if (registry.ctx().contains<GAME::Bounds>()) {
            auto& bounds = registry.ctx().get<GAME::Bounds>();
            
            if(enemyTrans.matrix.row4.x > bounds.left && enemyTrans.matrix.row4.x < bounds.right &&
               enemyTrans.matrix.row4.z > bounds.bottom && enemyTrans.matrix.row4.z < bounds.top)
            {
				auto& enemyHealth = registry.get<GAME::Health>(enemy);
                auto& cfg = registry.get<GAME::EnemyConfig>(enemy);
				enemyHealth.HP = 0;
                Gameplay::EnemyDeath(registry, cfg, GAME::DamageType::ScreenWipe);
			}
        }
	}
};

void Update_SideFighter(entt::registry& registry, entt::entity self)
{
    auto& sideFighter = registry.get<GAME::SideFighter>(self);

    double deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;

    if (registry.valid(sideFighter.player))
    {
        auto& playerTransform = registry.get<GAME::Transform>(sideFighter.player);
        auto& myTransform = registry.get<GAME::Transform>(self);

        float t = 1.0f - std::exp(-sideFighter.lerpSpeed * deltaTime);

        GW::MATH::GVector::LerpF(
            sideFighter.currentOffset,
            sideFighter.targetOffset,
            t,
            sideFighter.currentOffset
        );

        GW::MATH::GMatrix::TranslateLocalF(
            playerTransform.matrix,
            sideFighter.currentOffset,
            myTransform.matrix
        );

        if (sideFighter.currentOffset.z >= sideFighter.targetOffset.z - 0.1f) 
        {
			sideFighter.canShoot = true;
        }
    }
};

void OnSideFighterDeath(entt::registry& registry, entt::entity self)
{
    auto& sideFighter = registry.get<GAME::SideFighter>(self);
    if (registry.valid(sideFighter.player))
    {
        auto& sideFighterData = registry.get<GAME::HasSideFighters>(sideFighter.player);
        if (sideFighter.side == "LEFT")
        {
            sideFighterData.leftAlive = false;
        }
        else if (sideFighter.side == "RIGHT")
        {
            sideFighterData.rightAlive = false;
        }
    }
}

struct DropChance
{
    GAME::PowerUpType type;
    int weight;
};

GAME::PowerUpType GetRandomPowerUpType(entt::registry& registry)
{
    GAME::PowerUpType result;

    std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
    int SideFighter = config.get()->at("DropRate").at("SideFighter").as<int>();
	int MultiShot = config.get()->at("DropRate").at("MultiShot").as<int>();
	int ScreenWipe = config.get()->at("DropRate").at("ScreenWipe").as<int>();
	int ExtraLife = config.get()->at("DropRate").at("ExtraLife").as<int>();
	int BonusPoints = config.get()->at("DropRate").at("BonusPoints").as<int>();

    DropChance dropChances[] = {
        { GAME::PowerUpType::SideFighterPU, SideFighter },
        { GAME::PowerUpType::MultiShotPU, MultiShot },
        { GAME::PowerUpType::ScreenWipePU, ScreenWipe },
        //{ GAME::PowerUpType::ExtraLifePU, ExtraLife },
        //{ GAME::PowerUpType::BonusPointsPU, BonusPoints }
	};

	int dropSize = sizeof(dropChances) / sizeof(DropChance);

    int totalWeight = 0;
    for(int i = 0; i < dropSize; i++)
    {
        totalWeight += dropChances[i].weight;
	}

	int randomWeight = rand() % totalWeight;

    for (int i = 0; i < dropSize; i++) 
    {
        if(randomWeight < dropChances[i].weight)
        {
            result = dropChances[i].type;
            break;
		}

		randomWeight -= dropChances[i].weight;
    }

    return result;
};

