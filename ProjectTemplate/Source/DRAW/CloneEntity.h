#pragma once
#include "../GAME/GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"


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
    //Used to set movement of enemy based on formation style
    static GW::MATH::GVECTORF SetMovement(GAME::FormationStyle style) {
        GW::MATH::GVECTORF Move = { 0,0,0,1 };
        if (style == GAME::FormationStyle::WaveLeft) {  //starts from bottom left and makes a "wave" to the top of the screen
            Move = { 1.0f,0,0,1 };
        }
        else if (style == GAME::FormationStyle::WaveRight) {  //starts from bottom right and makes a "wave" to the top of the screen
            Move = { -1.0f,0,0,1 };
        }
        else if (style == GAME::FormationStyle::ArrowHeadDown) {  //starts from the top and makes arrow facing down 
            Move = { 0,0,-1.0f,1 };
        }
        else if (style == GAME::FormationStyle::ArrowHeadLeft) { //starts from the left and makes arrow facing right
            Move = { 1.0f,0,0,1 };
        }
        else if (style == GAME::FormationStyle::ArrowHeadRight) { //starts from the right and makes arrow facing left
            Move = { -1.0f,0,0,1 };
        }
        else if (style == GAME::FormationStyle::BigGuy) { //comes from the right as a mini boss
            Move = { 1.0f,0,0,1 };
        }
        else if (style == GAME::FormationStyle::TheFinal) { //comes from the left as the final boss
            Move = { -1.0f,0,0,1 };
        }
        return Move;
    }

    static entt::entity SpawnEnemy(entt::registry& registry,
        const DRAW::ModelManager& manager,
        const GAME::Transform& transform,
        const GAME::EnemyConfig& cfg, const float SpeedMult)
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

        //applying shooting enemytag
        int chancetoshoot = 90; // chance to become an enemy that fires bullets! example: 10 = 10% chance!
        if (config.Movement == GAME::FormationStyle::BigGuy || config.Movement == GAME::FormationStyle::TheFinal) {
            registry.emplace<GAME::ShootingEnemy>(enemy);
        }
        else {
            srand(time(NULL));
            int resultofroll = (rand() % 100);
            if (resultofroll <= chancetoshoot) {
                registry.emplace<GAME::ShootingEnemy>(enemy);
            }
        }
        //set the correct movement based on formation style
        vel.direction = SetMovement(config.Movement);
        //give the correct speed
        vel.direction.x *= cfg.speed * SpeedMult;
        vel.direction.z *= cfg.speed * SpeedMult;

        registry.emplace<GAME::EnemyExitSide>(enemy, GAME::GetExitSide(cfg.Movement));

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
        GW::MATH::GMatrix::IdentityF(enemyTransform.matrix);
        enemyTransform = transform;
        GW::MATH::GVECTORF scaler = {cfg.Scale, cfg.Scale, cfg.Scale, 1};
        GW::MATH::GMatrix::ScaleGlobalF(enemyTransform.matrix,scaler, enemyTransform.matrix);
        enemyTransform.matrix.row4.y = 0;

        // Scale up hitbox (for enemies too short for bullets to actually hit the model)
        collection.collider.extent.y *= 10.0f;

        for (auto mesh : collection.meshEntities)
        {
            auto& inst = registry.get<DRAW::GPUInstance>(mesh);
            inst.transform = enemyTransform.matrix;
        }

        // Add gameplay components
        registry.emplace<GAME::Health>(enemy, cfg.hitpoints);
        // Collidable tag
        registry.emplace<GAME::Collidable>(enemy);

		//Tag enemy as a power up carrier if the config says so, this will be used to determine if the enemy should drop a power up on death
        if(cfg.isPUCarrier)
        {
            registry.emplace<GAME::PUCarrier>(enemy);
		}

        return enemy;
    }
    //Token made for enemy to be queued up next
    static void EnemyTokenCreator(entt::registry& registry,
        const GAME::EnemyConfig& cfg, //enemy being used
        GAME::FormationStyle style,
        std::vector<GAME::EnemyToken>& CurrentList,
        GAME::Transform spawnPos,
        float spawnRate,
        float speedMult) {
        GAME::EnemyToken temp;
        std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
        temp.Enemy = cfg;
        temp.SpawnLocation = spawnPos;
        temp.SpawnRate = spawnRate;
        temp.SpeedMult = speedMult;
        temp.Style = style;
        CurrentList.push_back(temp);
    }


    //Spawns enemy formation with provided information
    static void SpawnFormation(entt::registry& registry,
        GAME::FormationStyle Style, //Formation Style
        int enemyCount, // how many enemies to spawn
        int spacing, // how far apart enmies are from each other
        float speed, //multipler to the enemies movement speed
        GAME::Transform StartLocation, //where to start the formation from
        const GAME::EnemyConfig& cfg, //enemy being used
        const DRAW::ModelManager& manager, //for knowing where to get the model from
        float SpawnDelay, // delay between enemy spawns for the formations
        std::vector<GAME::EnemyToken>& CurrentList //Enemy queue
        ) {
        double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;
        GAME::Transform LocationUpdates = StartLocation; //Used for each styles way of updating where the enemy spawns
        float SpawnTemp = SpawnDelay; //Used for certain styles so enemys spawn delay can be skipped sometimes.
        bool OnAnOff = false; //used to flip between stuff on certain styles
        float space = spacing; //used for custom spacing
        if (Style == GAME::FormationStyle::WaveLeft) {  //starts from bottom and makes a "wave" to the top of the screen
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnDelay, speed);
                GW::MATH::GVECTORF Spaced = { 0,0,spacing,1 };
                GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
            }
        }
        else if (Style == GAME::FormationStyle::WaveRight) {
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnDelay, speed);
                GW::MATH::GVECTORF Spaced = { 0,0,spacing,1 };
                GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
            }
        }
        else if (Style == GAME::FormationStyle::ArrowHeadDown) {
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnTemp, speed);
                if (i != 0) {
                    LocationUpdates = StartLocation;
                    space *= -1;
                    GW::MATH::GVECTORF Spaced = { space,0,0,1 };
                    GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
                    if (OnAnOff == false) {
                        SpawnTemp = SpawnDelay;
                        OnAnOff = true;
                        space -= spacing;
                    }
                    else {
                        SpawnTemp = 0;
                        OnAnOff = false;
                    }
                }
                else {
                    GW::MATH::GVECTORF Spaced = { space,0,0,1 };
                    GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
                    SpawnTemp = 0;
                }
            }
        }
        else if (Style == GAME::FormationStyle::ArrowHeadLeft) { 
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnTemp, speed);
                if (i != 0) {
                    LocationUpdates = StartLocation;
                    space *= -1;
                    GW::MATH::GVECTORF Spaced = { 0,0,space,1 };
                    GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
                    if (OnAnOff == false) {
                        SpawnTemp = SpawnDelay;
                        OnAnOff = true;
                        space -= spacing;
                    }
                    else {
                        SpawnTemp = 0;
                        OnAnOff = false;
                    }
                }
                else {
                    GW::MATH::GVECTORF Spaced = { 0,0,space,1 };
                    GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
                    SpawnTemp = 0;
                }
            }
        }
        else if (Style == GAME::FormationStyle::ArrowHeadRight) {
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnTemp, speed);
                if (i != 0) {
                    LocationUpdates = StartLocation;
                    space *= -1;
                    GW::MATH::GVECTORF Spaced = { 0,0,space,1 };
                    GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
                    if (OnAnOff == false) {
                        SpawnTemp = SpawnDelay;
                        OnAnOff = true;
                        space -= spacing;
                    }
                    else {
                        SpawnTemp = 0;
                        OnAnOff = false;
                    }
                }
                else {
                    GW::MATH::GVECTORF Spaced = { 0,0,space,1 };
                    GW::MATH::GMatrix::TranslateLocalF(LocationUpdates.matrix, Spaced, LocationUpdates.matrix);
                    SpawnTemp = 0;
                }
            }
        }
        else if (Style == GAME::FormationStyle::BigGuy) { //Movement based enemy, Just relies on Spawn Delay and speed!
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnDelay, speed);

            }
        }
        else if (Style == GAME::FormationStyle::TheFinal) {//Movement based enemy, Just relies on Spawn Delay!
            for (int i = 0; i < enemyCount; i++) {
                EnemyTokenCreator(registry, cfg, Style, CurrentList, LocationUpdates, SpawnDelay, speed);
            }
        }

        //Randomly select one of CurrentList's enemies to be a PowerUp Carrier
        std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
		int carrierChance = config.get()->at("DropRate").at("Carrier").as<int>();

		int randValue = rand() % 100;

        if (CurrentList.size() >= enemyCount && randValue <= carrierChance) 
        {
			int randomIndex = rand() % CurrentList.size();
			CurrentList[randomIndex].Enemy.isPUCarrier = true;
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
        return temp;
    }


    //Constantly called method that spawns enemies in the queue aka CurrentList
    static void SpawnEnemies(entt::registry& registry, const DRAW::ModelManager& manager, std::vector<GAME::EnemyToken>& CurrentList) {
        if (CurrentList.empty()) {
            return;
        }
        static float time = 0;
        static float currentSpawnDelay = 0;
        double dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;
        time += dt;
        if (time > currentSpawnDelay) {

            entt::entity enemy = registry.create();
            enemy = SpawnEnemy(registry,manager, CurrentList[0].SpawnLocation, CurrentList[0].Enemy,CurrentList[0].SpeedMult);
            currentSpawnDelay = CurrentList[0].SpawnRate;
            time = 0;
            CurrentList.erase(CurrentList.begin());
        }
    }

    static GAME::Transform GetOffscreenSpawn(
        const GAME::Bounds& bounds,
        GAME::FormationStyle style,
        float x, float z, // base position from JSON, used for the non-entry axis
        float margin = 10.0f) // how far past the edge to spawn
    {
        GAME::Transform spawn;
        GW::MATH::GMatrix::IdentityF(spawn.matrix);

        GW::MATH::GVECTORF position = { x, -4.0f, z, 0.0f };

        switch (style)
        {
        case GAME::FormationStyle::WaveLeft:
        case GAME::FormationStyle::ArrowHeadLeft:
        case GAME::FormationStyle::TheFinal:
            position.x = position.x - margin;  // off the left edge
            break;

        case GAME::FormationStyle::WaveRight:
        case GAME::FormationStyle::ArrowHeadRight:
        case GAME::FormationStyle::BigGuy:
            position.x = position.x + margin; // off the right edge
            break;

        case GAME::FormationStyle::ArrowHeadDown:
        default:
            position.z = position.z + margin;   // off the top edge
            break;
        }

        GW::MATH::GMatrix::TranslateGlobalF(spawn.matrix, position, spawn.matrix);
        return spawn;
    }

