#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "../CCL.h"
#include "../DRAW/CloneEntity.h"
#include "Gameplay/PowerUps/PowerUps.h"
#include "Gameplay/Gameplay.h"


void SideFighterFire(entt::registry& registry, entt::entity self, const GAME::Transform transform, GW::MATH::GVECTORF dir);

void Update_Player(entt::registry& registry, entt::entity self)
{
    // Check if player dies
    auto& health = registry.get<GAME::Health>(self);

    auto& soundCues = registry.ctx().get<GAME::SoundStorage>().soundCues;
    if (health.HP <= 0)
    {
        soundCues[2] = true;
        return;
    }

    auto& transform = registry.get<GAME::Transform>(self);
    auto& config = registry.ctx().get<UTIL::Config>().gameConfig;

    // Input + delta time
    auto& input = registry.ctx().get<UTIL::Input>();
    double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;

    // Load speed from config
    float speed = config->at("Player").at("speed").as<float>();

    float x = 0.0f;
    float z = 0.0f;

    float state = 0.0f;

    // WASD Input
    if (input.immediateInput.GetState(G_KEY_W, state) == GW::GReturn::SUCCESS && state > 0.0f)
        z += 1.0f;

    if (input.immediateInput.GetState(G_KEY_S, state) == GW::GReturn::SUCCESS && state > 0.0f)
        z -= 1.0f;

    if (input.immediateInput.GetState(G_KEY_D, state) == GW::GReturn::SUCCESS && state > 0.0f)
        x += 1.0f;

    if (input.immediateInput.GetState(G_KEY_A, state) == GW::GReturn::SUCCESS && state > 0.0f)
        x -= 1.0f;

    // Controller left stick input
    float leftX = 0.0f;
    float leftY = 0.0f;

    if (input.gamePads.GetState(0, G_LX_AXIS, leftX) == GW::GReturn::SUCCESS)
        x += leftX;

    if (input.gamePads.GetState(0, G_LY_AXIS, leftY) == GW::GReturn::SUCCESS)
        z += leftY;

    // Normalize diagonal movement
    float mag = sqrtf(x * x + z * z);
    if (mag > 0.001f) {
        x /= mag;
        z /= mag;
    }

    // Scale by speed and dt
    x *= speed * dt;
    z *= speed * dt;

    // Apply movement to the transform matrix
    GW::MATH::GMATRIXF newMat;
    if (registry.all_of<GAME::Roll>(self))
    {
        // During roll use global translation so rotation doesn't affect movement direction
        GW::MATH::GMatrix::TranslateGlobalF(
            transform.matrix,
            GW::MATH::GVECTORF{ x, 0.0f, z, 0.0f },
            newMat
        );
        // Also update the roll's start matrix position so the rotation stays centered
        auto& roll = registry.get<GAME::Roll>(self);
        roll.startMatrix.row4.x += x;
        roll.startMatrix.row4.z += z;
    }
    else
    {
        GW::MATH::GMatrix::TranslateLocalF(
            transform.matrix,
            GW::MATH::GVECTORF{ x, 0.0f, z, 0.0f },
            newMat
        );
    }
    transform.matrix = newMat;

    // Clamp player position to screen bounds
    if (registry.ctx().contains<GAME::Bounds>()) {
        auto& bounds = registry.ctx().get<GAME::Bounds>();
        transform.matrix.row4.x = std::clamp(transform.matrix.row4.x, bounds.left, bounds.right);
        transform.matrix.row4.z = std::clamp(transform.matrix.row4.z, bounds.bottom, bounds.top);
    }

    // Roll input
    float rollState = 0.0f;
    float bButton = 0.0f;

    input.gamePads.GetState(0, G_EAST_BTN, bButton);

    if ((input.immediateInput.GetState(G_KEY_LEFTSHIFT, rollState) == GW::GReturn::SUCCESS && rollState > 0.0f) || bButton > 0.0f)
    {
        if (!registry.all_of<GAME::Roll>(self) && !registry.all_of<GAME::Invuln>(self))
        {
            auto& charges = registry.get<GAME::RollCharges>(self);
            if (charges.charges > 0)
            {
                charges.charges--;
                auto& newRoll = registry.emplace<GAME::Roll>(self);
                newRoll.startMatrix = transform.matrix;
                // Invuln lasts the full roll duration
                auto& inv = registry.emplace_or_replace<GAME::Invuln>(self);
                inv.cooldown = newRoll.duration;
                inv.isRoll = true;
                std::cout << "Roll! Charges remaining: " << charges.charges << "\n";
            }
        }
    }
    //Regain charge if you get a certain score! Scales on difficulty!
    auto lmView = registry.view<GAME::LevelManager>();
    auto lmEntity = lmView.front();
    auto& lm = registry.get<GAME::LevelManager>(lmEntity);
    auto& charges = registry.get<GAME::RollCharges>(self);
    registry.ctx().get<ScoreSystem>().GetScore();
    float chargeRegencoolDown = 2; //to prevent more than one addition
    static int scoreupdate = 7000;
    int highscore = registry.ctx().get<ScoreSystem>().GetScore();
    if ( highscore > (scoreupdate + (lm.loops *4000))) {
        if (charges.charges < 5) {
            charges.charges++;
        }
        scoreupdate += 7000;
    }
    
    // Update roll visual tilt
    if (registry.all_of<GAME::Roll>(self))
    {
        auto& roll = registry.get<GAME::Roll>(self);
        roll.timeRemaining -= (float)dt;

        float progress = 1.0f - (roll.timeRemaining / roll.totalDuration);
        float angle = progress * G_PI_F * 2.0f;

        // Save current position before overwriting transform
        GW::MATH::GVECTORF currentPos = transform.matrix.row4;

        // Rotate from saved pre-roll orientation
        GW::MATH::GMatrix::RotateZLocalF(roll.startMatrix, angle, transform.matrix);

        // Restore current position so movement still applies correctly
        transform.matrix.row4 = currentPos;

        if (roll.timeRemaining <= 0.0f)
        {
            GW::MATH::GVECTORF finalPos = transform.matrix.row4;
            transform.matrix = roll.startMatrix;
            transform.matrix.row4 = finalPos;
            registry.remove<GAME::Roll>(self);
        }
    }

    // Firing cooldown
    float fireRate = config->at("Player").at("firerate").as<float>();
    if (registry.all_of<GAME::Firing>(self)) {
        auto& fire = registry.get<GAME::Firing>(self);
        fire.cooldown -= (float)dt;

        if (fire.cooldown <= 0.0f) {
            registry.remove<GAME::Firing>(self);
        }

        // If still cooling down, skip firing logic
        return;
    }

    // Check fire keys
    GW::MATH::GVECTORF dir = { 0, 0, 0, 0 };

    float fireState = 0.0f;
    float aButton = 0.0f;
    
    bool firePressed = false;

    // Controller
    input.gamePads.GetState(0, G_SOUTH_BTN, aButton);

    if ((input.immediateInput.GetState(G_KEY_UP, fireState) == GW::GReturn::SUCCESS && fireState > 0.0f) || aButton > 0.0f)
    {
        dir.z += 1.0f;
        firePressed = true;
    }

    float pressed = 0.0f;
    if (input.immediateInput.GetState(G_KEY_P, pressed) == GW::GReturn::SUCCESS && pressed > 0.0f) {
        //registry.emplace<GAME::Paused>(self);
    }

    if (input.immediateInput.GetState(G_KEY_0, pressed) == GW::GReturn::SUCCESS && pressed > 0.0f) {
		ClearPowerUPs(registry, self);
    }

    // Normalize
    float bulletMag = sqrtf(dir.x * dir.x + dir.z * dir.z);
    if (bulletMag > 0.001f) {
        dir.x /= bulletMag;
        dir.z /= bulletMag;
    }

    // Scale by bullet speed from config
    float bulletSpeed = config->at("Bullet").at("speed").as<float>();
    dir.x *= bulletSpeed;
    dir.z *= bulletSpeed;

    if (firePressed)
    {
        auto SpawnBullet = [&](GW::MATH::GVECTORF bulletVelocity)
        {
            entt::entity bullet = registry.create();

            soundCues[0] = true;

            // Add components
            registry.emplace<GAME::Velocity>(bullet, bulletVelocity);
            registry.emplace<GAME::Bullet>(bullet);
            registry.emplace<GAME::Collidable>(bullet);
            auto& bulletTransform = registry.emplace<GAME::Transform>(bullet);

            // Clone meshes
            auto& manager = registry.ctx().get<DRAW::ModelManager>();
            auto& bulletCollection = registry.emplace<DRAW::MeshCollection>(bullet);

            // Clone meshes 
            CloneModelToEntity(
                registry,
                manager.collections["BlueBullet"],
                bulletCollection,
                bulletTransform
            );

            // Override the transform
            bulletTransform.matrix = transform.matrix;
            bulletTransform.matrix.row1.x *= 0.75f;
            bulletTransform.matrix.row2.y *= 0.75f;
            bulletTransform.matrix.row3.z *= 0.75f;

            for (auto mesh : bulletCollection.meshEntities) {
                auto& inst = registry.get<DRAW::GPUInstance>(mesh);
                inst.transform = bulletTransform.matrix;
            }
        };

        if (auto* multiShot = registry.try_get<GAME::MultiShot>(self))
        {
			// Multi-shot power-up logic: spawn bullets in multiple directions
            for (auto& direction : multiShot->directions)
            {
                GW::MATH::GVECTORF finalVel = {
                    direction.x * bulletSpeed,
                    0.0f,
                    direction.z * bulletSpeed,
                    0.0f
                };
                SpawnBullet(finalVel);
            }
        }
        else
        {
            // Standard single shot
            SpawnBullet(dir);
        }


        SideFighterFire(registry, self, transform, dir);

        // Cooldown
        registry.emplace<GAME::Firing>(self, fireRate);
    }

}

void SideFighterFire(entt::registry& registry, entt::entity self, const GAME::Transform transform, GW::MATH::GVECTORF dir)
{
    auto wingmanView = registry.view<GAME::SideFighter>();

    for (auto wingmanEntity : wingmanView)
    {
        auto& fighterData = registry.get<GAME::SideFighter>(wingmanEntity);

        if(fighterData.canShoot == false)
        {
            continue;
        }

        if (fighterData.player == self)
        {
            entt::entity bullet = registry.create();

            auto& vel = registry.emplace<GAME::Velocity>(bullet);
            vel.direction = dir;

            registry.emplace<GAME::Bullet>(bullet);
            registry.emplace<GAME::Collidable>(bullet);

            auto& bulletTransform = registry.emplace<GAME::Transform>(bullet);

            auto& manager = registry.ctx().get<DRAW::ModelManager>();
            auto& bulletCollection = registry.emplace<DRAW::MeshCollection>(bullet);

            CloneModelToEntity(
                registry,
                manager.collections["BlueBullet"],
                bulletCollection,
                bulletTransform
            );

            GW::MATH::GMatrix::TranslateGlobalF(
                transform.matrix,
                fighterData.targetOffset,
                bulletTransform.matrix
			);
            bulletTransform.matrix.row1.x *= 0.75f;
            bulletTransform.matrix.row2.y *= 0.75f;
            bulletTransform.matrix.row3.z *= 0.75f;

            for (auto mesh : bulletCollection.meshEntities)
            {
                auto& inst = registry.get<DRAW::GPUInstance>(mesh);
                inst.transform = bulletTransform.matrix;
            }
        }
    }
}

void EnemyUPDATES(entt::registry& registry) {
    Gameplay::EnemyFiringUpdates(registry);
    Gameplay::EnemyMovementUpdates(registry);
}
CONNECT_COMPONENT_LOGIC()
{
    registry.on_update<GAME::Player>().connect<Update_Player>();
    registry.on_update<GAME::SideFighter>().connect<Update_SideFighter>();
    registry.on_update<GAME::Enemy>().connect<EnemyUPDATES>();
}