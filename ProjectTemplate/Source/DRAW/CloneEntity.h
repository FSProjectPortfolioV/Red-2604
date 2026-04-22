#pragma once
#include "../GAME/GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "../GAME/EnemyManger.h"




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
        const GAME::EnemyConfig& cfg, const float SpeedMult, const float spawnInvul)
    {
        // Create entity
        entt::entity enemy = registry.create();

        // Add components
        registry.emplace<GAME::Enemy>(enemy);
        registry.emplace<GAME::EnemyConfig>(enemy);
        auto& config = registry.get<GAME::EnemyConfig>(enemy);
        config = cfg;
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
        registry.emplace<GAME::Invuln>(enemy);
        auto& invul = registry.get<GAME::Invuln>(enemy);
        invul.cooldown = spawnInvul;
        // Collidable tag
        registry.emplace<GAME::Collidable>(enemy);

        return enemy;
    }
    //Token made for enemy to be queued up next
    static void EnemyTokenCreator(entt::registry& registry,
        const GAME::EnemyConfig& cfg, //enemy being used
        GAME::FormationStyle style,
        std::vector<GAME::EnemyToken>& CurrentList,
        GAME::Transform spawnPos,
        float spawnRate,
        float speedMult,
        int UsageCost) {
        GAME::EnemyToken temp;
        std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
        temp.Enemy = cfg;
        temp.SpawnLocation = spawnPos;
        temp.SpawnRate = spawnRate;
        temp.SpeedMult = speedMult;
        temp.Style = style;
        temp.UsageCost = UsageCost;
        CurrentList.push_back(temp);
    }


    //Spawns enemy formation with provided information
    static void SpawnFormation(entt::registry& registry,
        GAME::FormationStyle Style, //Formation Style
        int enemyCount, // how many enemies to spawn
        int spacing, // how far apart enmies are from each other
        float speed, //multipler to the enemies movement speed
        GAME::Transform StartLocation, //where to start the formation from
        GAME::EnemyConfig& cfg, //enemy being used
        const DRAW::ModelManager& manager, //for knowing where to get the model from
        float SpawnDelay, // delay between enemy spawns for the formations
        std::vector<GAME::EnemyToken>& CurrentList, //Enemy queue
        int UsageCost //PER ENEMY!!!
        ) {
        double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;
        GAME::Transform LocationUpdates = StartLocation; //Used for each styles way of updating where the enemy spawns
        if (Style == GAME::FormationStyle::WaveLeft) {  //starts from bottom and makes a "wave" to the top of the screen
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnDelay, speed, UsageCost);
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
    //keep all enemies using the same type of data with different stats, only the name changing. //Edit for real enemy stats
    static GAME::EnemyConfig EnemyCFGCreator(entt::registry& registry,std::string& dataname,GAME::FormationStyle style) {
        GAME::EnemyConfig temp;
        std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
        temp.hitpoints = (*config).at(dataname).at("hitpoints").as<int>();
        temp.modelName = (*config).at(dataname).at("model").as<std::string>();
        temp.Movement = style;
        temp.speed = (*config).at(dataname).at("speed").as<float>();
        temp.Score = (*config).at(dataname).at("score").as<int>();
        temp.fireRate = (*config).at(dataname).at("firerate").as<float>();
        temp.Spawn.cooldown = (*config).at(dataname).at("invul").as<float>();
        return temp;
    }


    //Constalyu called method that spawns enemies in the queue aka CurrentList
    static void SpawnEnemies(entt::registry& registry, const DRAW::ModelManager& manager, std::vector<GAME::EnemyToken>& CurrentList, int& RemainingCost) {
        if (CurrentList.empty()) {
            return;
        }
        static float time = 0;
        static float currentSpawnDelay = 0;
        double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;
        time += dt;
        if (time > currentSpawnDelay && RemainingCost - CurrentList[0].UsageCost > 0) {

            RemainingCost = RemainingCost - CurrentList[0].UsageCost;
            entt::entity enemy = registry.create();
            enemy = SpawnEnemy(registry,manager, CurrentList[0].SpawnLocation, CurrentList[0].Enemy,CurrentList[0].SpeedMult, CurrentList[0].Enemy.Spawn.cooldown);
            currentSpawnDelay = CurrentList[0].SpawnRate;
            time = 0;
            CurrentList.erase(CurrentList.begin());
        }
    }