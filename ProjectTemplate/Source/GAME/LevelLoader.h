#pragma once
#include "EnemyManger.h"
#include "GameComponents.h"

namespace GAME 
{
	static FormationStyle FormationFromString(const std::string& s);

	LevelData LoadLevelData(const std::string& filePath);
}