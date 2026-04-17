#ifndef UTILITIES_H_
#define UTILITIES_H_

#include "GameConfig.h"

namespace UTIL
{
	struct Config
	{
		std::shared_ptr<GameConfig> gameConfig = std::make_shared<GameConfig>();
	};

	struct DeltaTime
	{
		double dtSec;
	};

	struct Input
	{
		GW::INPUT::GController gamePads; // controller support
		GW::INPUT::GInput immediateInput; // twitch keybaord/mouse
		GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse
	};

	/// Method declarations

	/// Creates a normalized vector pointing in a random direction on the X/Z plane
	GW::MATH::GVECTORF GetRandomVelocityVector();

	// Generate a random float between min and max
	float RandomFloat(float min, float max);

} // namespace UTIL
#endif // !UTILITIES_H_