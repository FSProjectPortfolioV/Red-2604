#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "../CCL.h"
#include "../DRAW/CloneEntity.h"
#include "Gameplay/PowerUps/PowerUps.h"


void SideFighterFire(entt::registry& registry, entt::entity self, const GAME::Transform transform, GW::MATH::GVECTORF dir);

void Update_Player(entt::registry& registry, entt::entity self)
{
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
    GW::MATH::GMatrix::TranslateLocalF(
        transform.matrix,
        GW::MATH::GVECTORF{ x, 0.0f, z, 0.0f },
        newMat
    );
    transform.matrix = newMat;

    // Clamp player position to screen bounds
    if (registry.ctx().contains<GAME::Bounds>()) {
        auto& bounds = registry.ctx().get<GAME::Bounds>();
        transform.matrix.row4.x = std::clamp(transform.matrix.row4.x, bounds.left, bounds.right);
        transform.matrix.row4.z = std::clamp(transform.matrix.row4.z, bounds.bottom, bounds.top);
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

    bool firePressed = false;

    if (input.immediateInput.GetState(G_KEY_UP, fireState) == GW::GReturn::SUCCESS && fireState > 0.0f)
    {
        dir.z += 1.0f;
        firePressed = true;
    }
   

    if (input.immediateInput.GetState(G_KEY_DOWN, fireState) == GW::GReturn::SUCCESS && fireState > 0.0f)
    {
        dir.z -= 1.0f;
        //firePressed = true;
    }

    if (input.immediateInput.GetState(G_KEY_RIGHT, fireState) == GW::GReturn::SUCCESS && fireState > 0.0f)
    {
        dir.x += 1.0f;
        //firePressed = true;
    }

    if (input.immediateInput.GetState(G_KEY_LEFT, fireState) == GW::GReturn::SUCCESS && fireState > 0.0f)
    {
        dir.x -= 1.0f;
        //firePressed = true;
    }

    float pressed = 0.0f;
    if (input.immediateInput.GetState(G_KEY_P, pressed) == GW::GReturn::SUCCESS && pressed > 0.0f) {
        //registry.emplace<GAME::Paused>(self);
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

    if (firePressed) {
        entt::entity bullet = registry.create();

        // Add components
        auto& vel = registry.emplace<GAME::Velocity>(bullet);
        vel.direction = dir;

        registry.emplace<GAME::Bullet>(bullet);
        registry.emplace<GAME::Collidable>(bullet);

        auto& bulletTransform = registry.emplace<GAME::Transform>(bullet);

        auto& manager = registry.ctx().get<DRAW::ModelManager>();
        auto& bulletCollection = registry.emplace<DRAW::MeshCollection>(bullet);

        // Clone meshes 
        CloneModelToEntity(
            registry,
            manager.collections["Bullet"],
            bulletCollection,
            bulletTransform
        );

        // Override the transform
        bulletTransform.matrix = transform.matrix;

        for (auto mesh : bulletCollection.meshEntities)
        {
            auto& inst = registry.get<DRAW::GPUInstance>(mesh);
            inst.transform = bulletTransform.matrix;
        }

        SideFighterFire(registry, self, transform, dir);

        // Cooldown
        registry.emplace<GAME::Firing>(self, fireRate);
    }

    // Temporary key press logic to test power up spawning
    // Gets the event cache to read buffered input events
    auto& pressEvents = registry.ctx().get<GW::CORE::GEventCache>();
    GW::GEvent event;

    while (+pressEvents.Pop(event))
    {
        GW::INPUT::GBufferedInput::Events inputEvent;
        GW::INPUT::GBufferedInput::EVENT_DATA inputData;

        if (+event.Read(inputEvent, inputData))
        {
            // Check if a key was specifically pressed down and if it's 'C'
            if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_C)
            {
				SpawnPowerUp(registry, transform.matrix);
                
            }
        }
    }
}

void SideFighterFire(entt::registry& registry, entt::entity self, const GAME::Transform transform, GW::MATH::GVECTORF dir)
{
    auto wingmanView = registry.view<GAME::SideFighter>();

    for (auto wingmanEntity : wingmanView)
    {
        auto& fighterData = registry.get<GAME::SideFighter>(wingmanEntity);

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
                manager.collections["Bullet"],
                bulletCollection,
                bulletTransform
            );

            GW::MATH::GMatrix::TranslateGlobalF(
                transform.matrix,
                fighterData.offset,
                bulletTransform.matrix
			);

            for (auto mesh : bulletCollection.meshEntities)
            {
                auto& inst = registry.get<DRAW::GPUInstance>(mesh);
                inst.transform = bulletTransform.matrix;
            }
        }
    }
}

CONNECT_COMPONENT_LOGIC()
{
    registry.on_update<GAME::Player>().connect<Update_Player>();
    registry.on_update<GAME::SideFighter>().connect<Update_SideFighter>();
}