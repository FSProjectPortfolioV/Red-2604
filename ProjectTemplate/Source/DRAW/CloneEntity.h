#pragma once
#include "../GAME/GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "../GAME/EnemyManger.h"


    struct EnemyConfig 
    {
        float speed;
        int hitpoints;
        int initialShatterCount;
        int shatterAmount;
        float shatterScale;
        std::string modelName;
        GAME::FormationStyle Movement; //used for knowing how to move the enemy
        GAME::Invuln Spawn; //used for preventing accidental on spawn deleteion. 
    };

    static void CloneModelToEntity(
        entt::registry& registry,
        const DRAW::MeshCollection& source,
        DRAW::MeshCollection& dest,
        GAME::Transform& transformOut)
    {

        dest.meshEntities.clear();

        bool first = true;

        for (auto mesh : source.meshEntities)
        {
            if (!registry.valid(mesh))
                continue;

            entt::entity clone = registry.create();

            // Copy GPUInstance
            const auto& srcInst = registry.get<DRAW::GPUInstance>(mesh);
            registry.emplace<DRAW::GPUInstance>(clone, srcInst);

            // Copy GeometryData
            const auto& srcGeo = registry.get<DRAW::GeometryData>(mesh);
            registry.emplace<DRAW::GeometryData>(clone, srcGeo);

            // Add to destination collection
            dest.meshEntities.push_back(clone);

            // First mesh defines the transform
            if (first)
            {
                transformOut.matrix = srcInst.transform;
                first = false;
            }
        }
        dest.collider = source.collider;
    }

    static entt::entity SpawnEnemy(entt::registry& registry,
        const DRAW::ModelManager& manager, 
        const GAME::Transform& transform, 
        const EnemyConfig& cfg, const float SpeedMult)
    {
        // Create entity
        entt::entity enemy = registry.create();

        // Add components
        registry.emplace<GAME::Enemy>(enemy);

        // Velocity
        auto& vel = registry.emplace<GAME::Velocity>(enemy);
        GW::MATH::GVECTORF hardcodedmovement = { 1.0f,0.0f,0.0f,1.0f }; //THIS IS TEMP FOR ONCE I ADD AN ENEMY MOVEMENT SYSTEM
        vel.direction = hardcodedmovement;
        vel.direction.x *= cfg.speed * SpeedMult;
        vel.direction.z *= cfg.speed * SpeedMult;

        // MeshCollection + Transform
        auto& collection = registry.emplace<DRAW::MeshCollection>(enemy);
        auto& enemyTransform = registry.emplace<GAME::Transform>(enemy);

        // Clone model
        CloneModelToEntity(
            registry,
            manager.collections.at(cfg.modelName),
            collection,
            enemyTransform
        );

        // Override transform with the one passed in
        enemyTransform = transform;

        for (auto mesh : collection.meshEntities)
        {
            auto& inst = registry.get<DRAW::GPUInstance>(mesh);
            inst.transform = enemyTransform.matrix;
        }

        // Add gameplay components
        registry.emplace<GAME::Health>(enemy, cfg.hitpoints);

        if (cfg.initialShatterCount > 0)
        {
            registry.emplace<GAME::Shatters>(
                enemy,
                cfg.initialShatterCount,
                cfg.shatterAmount,
                cfg.shatterScale
            );
        }

        // Collidable tag
        registry.emplace<GAME::Collidable>(enemy);

        return enemy;
    }

    static entt::entity SpawnEnemyShatterChild(
        entt::registry& registry,
        const DRAW::ModelManager& manager,
        const GAME::Transform& transform,
        int remaining,
        int clones,
        float scaleDown,
        float speed,
        const std::string& modelName)
    {
        entt::entity enemy = registry.create();

        registry.emplace<GAME::Enemy>(enemy);

        // Velocity
        auto& vel = registry.emplace<GAME::Velocity>(enemy);
        GW::MATH::GVECTORF hardcodedmovement = { 1.0f,0.0f,0.0f,1.0f }; //THIS IS TEMP FOR ONCE I ADD AN ENEMY MOVEMENT SYSTEM
        vel.direction = hardcodedmovement; 
        vel.direction.x *= speed;
        vel.direction.z *= speed;

        // Mesh + transform
        auto& collection = registry.emplace<DRAW::MeshCollection>(enemy);
        auto& enemyTransform = registry.emplace<GAME::Transform>(enemy);

        CloneModelToEntity(registry, manager.collections.at(modelName), collection, enemyTransform);

        enemyTransform = transform;

        for (auto mesh : collection.meshEntities)
        {
            auto& inst = registry.get<DRAW::GPUInstance>(mesh);
            inst.transform = enemyTransform.matrix;
        }

        // Health always resets
        registry.emplace<GAME::Health>(enemy, 1);

        // Only add Shatters if remaining > 0
        if (remaining > 0)
            registry.emplace<GAME::Shatters>(enemy, remaining, clones, scaleDown);

        registry.emplace<GAME::Collidable>(enemy);

        return enemy;
    }


    static void SpawnFormation(entt::registry& registry,
        GAME::FormationStyle Style, //Formation Style
        int enemyCount, // how many enemies to spawn
        int spacing , // how far apart enmies are from each other
        float speed, //multipler to the enemies movement speed
        GAME::Transform StartLocation, //where to start the formation form
        const EnemyConfig& cfg, //enemy being used
        const DRAW::ModelManager& manager, //for knowing where to get the model from
        float SpawnDelay // delay between enemy spawns for the formations
        ) {
        double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;
        GAME::Transform LocationUpdates = StartLocation; //Used for each styles way of updating where the enemy spawns
        if (Style == GAME::FormationStyle::WaveLeft) {  //starts from bottom and makes a "wave" to the top of the screen
            for (int i = 0; i < enemyCount; i++) {
                //spawndelay!
                entt::entity enemy = registry.create();
                enemy = SpawnEnemy(registry, manager, LocationUpdates, cfg, speed);
                GW::MATH::GVECTORF Spaced = { 0,0,spacing,1 };
                GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced,LocationUpdates.matrix);
            }
        }
        else if (Style == GAME::FormationStyle::WaveRight) {

        }
        else if (Style == GAME::FormationStyle::ArrowHeadDown) {

        }
        else if (Style == GAME::FormationStyle::ArrowHeadLeft) {

        }
        else if (Style == GAME::FormationStyle::ArrowHeadRight) {

        }
        else if (Style == GAME::FormationStyle::BigGuy) {

        }
        else if (Style == GAME::FormationStyle::TheFinal) {

        }
    }

    static EnemyConfig EnemyCFGCreator(entt::registry& registry,std::string& dataname,GAME::FormationStyle style) {
        EnemyConfig temp;
        std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;


        temp.hitpoints = (*config).at(dataname).at("hitpoints").as<int>();
        temp.modelName = (*config).at(dataname).at("model").as<std::string>();
        temp.initialShatterCount = (*config).at(dataname).at("hitpoints").as<int>();
        temp.Movement = style;
        temp.shatterAmount = (*config).at(dataname).at("shatterAmount").as<int>();
        temp.shatterScale = (*config).at(dataname).at("shatterScale").as<float>();
        temp.speed = (*config).at(dataname).at("speed").as<float>();
        return temp;
    }