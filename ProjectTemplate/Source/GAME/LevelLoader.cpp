#include "../UTIL/json.hpp"
#include "LevelLoader.h"
#include <fstream>
#include <iostream>

    static GAME::FormationStyle GAME::FormationFromString(const std::string& s)
    {
        if (s == "WaveLeft")       return FormationStyle::WaveLeft;
        if (s == "WaveRight")      return FormationStyle::WaveRight;
        if (s == "ArrowHeadDown")  return FormationStyle::ArrowHeadDown;
        if (s == "ArrowHeadLeft")  return FormationStyle::ArrowHeadLeft;
        if (s == "ArrowHeadRight") return FormationStyle::ArrowHeadRight;
        if (s == "BigGuy")         return FormationStyle::BigGuy;
        if (s == "TheFinal")       return FormationStyle::TheFinal;
        return FormationStyle::WaveLeft;
    }

    GAME::LevelData GAME::LoadLevelData(const std::string& filePath)
    {
        GAME::LevelData result;

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "[LevelLoader] Failed to open: " << filePath << "\n";
            return result;
        }

        nlohmann::json j;
        file >> j;

        result.duration = j.at("duration").get<float>();

        for (auto& waveJson : j.at("waves"))
        {
            GAME::Wave wave;
            wave.triggerTime = waveJson.at("triggerTime").get<float>();

            auto& tokenJson = waveJson.at("token");
            auto& enemyJson = tokenJson.at("enemy");

            wave.token.Style = GAME::FormationFromString(tokenJson.at("style").get<std::string>());
            wave.token.SpawnRate = tokenJson.at("spawnRate").get<float>();
            wave.token.SpeedMult = tokenJson.at("speedMult").get<float>();

            // Build spawn location matrix from JSON position
            auto& loc = tokenJson.at("spawnLocation");
            GW::MATH::GVECTORF position = {
                loc.at("x").get<float>(),
                loc.at("y").get<float>(),
                loc.at("z").get<float>(),
                0.0f
            };
            GW::MATH::GMatrix::IdentityF(wave.token.SpawnLocation.matrix);
            GW::MATH::GMatrix::TranslateGlobalF(
                wave.token.SpawnLocation.matrix,
                position,
                wave.token.SpawnLocation.matrix
            );

            // Enemy config
            wave.token.Enemy.modelName = enemyJson.at("modelName").get<std::string>();
            wave.token.Enemy.speed = enemyJson.at("speed").get<float>();
            wave.token.Enemy.hitpoints = enemyJson.at("hitpoints").get<int>();
            wave.token.Enemy.Scale = enemyJson.at("scale").get<float>();
            wave.token.Enemy.Score = enemyJson.at("score").get<int>();
            wave.token.Enemy.fireRate = enemyJson.at("fireRate").get<float>();
            wave.token.Enemy.Movement = GAME::FormationFromString(enemyJson.at("movement").get<std::string>());

            result.waves.push_back(wave);
        }

        return result;
    }