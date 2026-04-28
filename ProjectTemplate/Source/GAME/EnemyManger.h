#include "../GAME/GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "../CCL.h"
#include "../DRAW/CloneEntity.h"
#include "../GAME/Gameplay/PowerUps/PowerUps.h"

void EnemyUPDATES(entt::registry& registry) {
    Gameplay::EnemyFiringUpdates(registry);
    Gameplay::EnemyMovementUpdates(registry);
}

CONNECT_COMPONENT_LOGIC()
{
    registry.on_update<GAME::Enemy>().connect<EnemyUPDATES>();
}