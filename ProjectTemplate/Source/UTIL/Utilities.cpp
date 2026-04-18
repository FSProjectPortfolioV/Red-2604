#include "Utilities.h"
#include "../CCL.h"
namespace UTIL
{
	GW::MATH::GVECTORF GetRandomVelocityVector()
	{
		GW::MATH::GVECTORF vel = {float((rand() % 20) - 10), 0.0f, float((rand() % 20) - 10)};
		if (vel.x <= 0.0f && vel.x > -1.0f)
			vel.x = -1.0f;
		else if (vel.x >= 0.0f && vel.x < 1.0f)
			vel.x = 1.0f;

		if (vel.z <= 0.0f && vel.z > -1.0f)
			vel.z = -1.0f;
		else if (vel.z >= 0.0f && vel.z < 1.0f)
			vel.z = 1.0f;

		GW::MATH::GVector::NormalizeF(vel, vel);

		return vel;
	}

	float RandomFloat(float min, float max)
	{
		float random = ((float)rand()) / (float)RAND_MAX;
		float range = max - min;
		return (random * range) + min;
	}

	GW::MATH::GMATRIXF OrthoLH(
		float left, float right,
		float bottom, float top,
		float nearZ, float farZ)
	{
		GW::MATH::GMATRIXF m = {};

		m.row1.x = 2.0f / (right - left);
		m.row2.y = 2.0f / (top - bottom);
		m.row3.z = 1.0f / (farZ - nearZ);

		m.row4.x = -(left + right) / (right - left);
		m.row4.y = -(top + bottom) / (top - bottom);
		m.row4.z = -nearZ / (farZ - nearZ);
		m.row4.w = 1.0f;

		return m;
	}

} // namespace UTIL