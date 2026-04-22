#pragma once

struct Physics
{
	static void Velocity(entt::registry& registry, float dt);
	static void Collision(entt::registry& registry);
	static void WorldLimit(entt::registry& registry);
};

