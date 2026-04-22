#include "PowerUps.h"
#include "../../../CCL.h"


void SpawnPowerUp(entt::registry& registry, GAME::PowerUpType type, const GW::MATH::GMATRIXF& transform)
{
    entt::entity powerUp = registry.create();
    auto& vel = registry.emplace<GAME::Velocity>(powerUp);

    GW::MATH::GVECTORF powerUpDir = { 0, 0, -1, 0 };
    powerUpDir.z *= 5.0f;
    vel.direction = powerUpDir;

	//Temporary, should be set based on the type of powerup
    registry.emplace<GAME::PowerUp>(powerUp, GAME::PowerUpType::SideFighterPU);
    registry.emplace<GAME::Collidable>(powerUp);

    auto& powerUpTransform = registry.emplace<GAME::Transform>(powerUp);
    auto& manager = registry.ctx().get<DRAW::ModelManager>();
    auto& powerUpCollection = registry.emplace<DRAW::MeshCollection>(powerUp);

    CloneModelToEntity(
        registry,
        manager.collections["SideFighterPU"],
        powerUpCollection,
        powerUpTransform
    );

    // Spawn it in random position
    float spawnRange = 50.0f;
    float randX = ((float)rand() / RAND_MAX) * spawnRange - (spawnRange / 2.0f);
    float randZ = ((float)rand() / RAND_MAX) * spawnRange - (spawnRange / 2.0f);

    GW::MATH::GMatrix::TranslateGlobalF(
        powerUpTransform.matrix,
        GW::MATH::GVECTORF{ randX, 0.0f, randZ, 0.0f },
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
};

void Update_SideFighter(entt::registry& registry, entt::entity self)
{
    auto& sideFighter = registry.get<GAME::SideFighter>(self);

    double deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;

    if (registry.valid(sideFighter.player))
    {
        auto& playerTransform = registry.get<GAME::Transform>(sideFighter.player);
        auto& myTransform = registry.get<GAME::Transform>(self);

        GW::MATH::GVector::LerpF(
            sideFighter.currentOffset,
            sideFighter.targetOffset,
            deltaTime * sideFighter.lerpSpeed,
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
};

