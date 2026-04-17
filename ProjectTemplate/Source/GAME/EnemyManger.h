#include "../../gateware-main/Gateware.h"
#define GAME_COMPONENTS_H_
namespace GAME
{
	int EnemyCount; //Current Alive enemies, could possibly be used for levels token count?
	int EnemyTotal; //How many enemies have spawn total
	int EnemyDestroyed;//How many enemies were destoryed by the player
	int EnemyFleed; //How many enemies weren't destoryed by the player and just flew offscreen

	enum FormationStyle { //used by enemies to know to move //used by level to know what to spawn
		WaveLeft = 0,
		WaveRight = 1,
		ArrowHeadDown = 2,
		ArrowHeadLeft = 3,
		ArrowHeadRight = 4,
		BigGuy = 5,
		TheFinal =6,
	};
	
	struct FORMATIONS { //main way level should try spawning enmies
		FormationStyle Form;
		int UsageCost;
	};


}
