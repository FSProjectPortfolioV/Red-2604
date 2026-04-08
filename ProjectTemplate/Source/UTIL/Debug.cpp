#include "Debug.h"
#include "Utilities.h"
#include "../GAME/GameComponents.h"
#include "../DRAW/DrawComponents.h"

void DebugOverlay::Draw(entt::registry& registry)
{
    auto& dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;

    size_t enemies = registry.view<GAME::Enemy>().size();
    size_t bullets = registry.view<GAME::Bullet>().size();
    size_t players = registry.view<GAME::Player>().size();

    // Print to console for now. Maybe have it actually be a visual overlay once we get some UI up and running?
    std::cout << "FPS: " << (1.0 / dt)
        << " | Enemies: " << enemies
        << " | Bullets: " << bullets
        << "\n";
}

void DebugDraw::Colliders(entt::registry& registry)
{
    using namespace GW::MATH;

    auto view = registry.view<GAME::Transform, DRAW::MeshCollection, GAME::Collidable>();

    for (auto e : view)
    {
        auto& t = view.get<GAME::Transform>(e).matrix;
        auto col = view.get<DRAW::MeshCollection>(e).collider;

        // Apply scale, rotation, translation
        GVECTORF scale;
        GMatrix::GetScaleF(t, scale);
        col.extent.x *= scale.x;
        col.extent.y *= scale.y;
        col.extent.z *= scale.z;
        col.center.w = 1;

        GMatrix::VectorXMatrixF(t, col.center, col.center);

        GQUATERNIONF qA;
        GQuaternion::SetByMatrixF(t, qA);
        GQuaternion::MultiplyQuaternionF(qA, col.rotation, col.rotation);

        // Draw lines for the 8 corners
        DebugRenderer::DrawOBB(col);
    }
}

void DebugRenderer::DrawOBB(GW::MATH::GOBBF& obb)
{
    using namespace GW::MATH;

    // Extract quaternion components
    float x = obb.rotation.x;
    float y = obb.rotation.y;
    float z = obb.rotation.z;
    float w = obb.rotation.w;

    // Compute rotation matrix from quaternion
    GMATRIXF rot;
    rot.row1 = { 1 - 2 * (y * y + z * z), 2 * (x * y + w * z),     2 * (x * z - w * y),     0 };
    rot.row2 = { 2 * (x * y - w * z),     1 - 2 * (x * x + z * z), 2 * (y * z + w * x),     0 };
    rot.row3 = { 2 * (x * z + w * y),     2 * (y * z - w * x),     1 - 2 * (x * x + y * y), 0 };
    rot.row4 = { 0, 0, 0, 1 };

    // Extract basis vectors from rotation matrix
    GVECTORF right = { rot.row1.x, rot.row2.x, rot.row3.x, 0 };
    GVECTORF up = { rot.row1.y, rot.row2.y, rot.row3.y, 0 };
    GVECTORF forward = { rot.row1.z, rot.row2.z, rot.row3.z, 0 };

    // Scale basis vectors by extents
    GVector::ScaleF(right, obb.extent.x, right);
    GVector::ScaleF(up, obb.extent.y, up);
    GVector::ScaleF(forward, obb.extent.z, forward);

    // Compute corners
    GVECTORF c = obb.center;
    GVECTORF corners[8];

    // Bottom face
    GVector::SubtractVectorF(c, right, corners[0]);
    GVector::SubtractVectorF(corners[0], forward, corners[0]);

    GVector::AddVectorF(c, right, corners[1]);
    GVector::SubtractVectorF(corners[1], forward, corners[1]);

    GVector::SubtractVectorF(c, right, corners[2]);
    GVector::AddVectorF(corners[2], forward, corners[2]);

    GVector::AddVectorF(c, right, corners[3]);
    GVector::AddVectorF(corners[3], forward, corners[3]);

    // Top face = bottom + up
    for (int i = 0; i < 4; i++)
        GVector::AddVectorF(corners[i], up, corners[i + 4]);

    // Draw edges
    auto DrawEdge = [&](int a, int b)
        {
            DebugRenderer::DrawLine(corners[a], corners[b]);
        };

    // Bottom
    DrawEdge(0, 1);
    DrawEdge(1, 3);
    DrawEdge(3, 2);
    DrawEdge(2, 0);

    // Top
    DrawEdge(4, 5);
    DrawEdge(5, 7);
    DrawEdge(7, 6);
    DrawEdge(6, 4);

    // Vertical edges
    DrawEdge(0, 4);
    DrawEdge(1, 5);
    DrawEdge(2, 6);
    DrawEdge(3, 7);
}



void DebugLog::Info(const std::string& msg)
{
    std::cout << "[INFO] " << msg << "\n";
}

void DebugLog::Warn(const std::string& msg)
{
    std::cout << "[WARN] " << msg << "\n";
}

void DebugLog::Error(const std::string& msg)
{
    std::cerr << "[ERROR] " << msg << "\n";
}

void DebugInspector::Inspect(entt::registry& registry)
{
    using namespace GAME;

    if (!registry.valid(selected))
    {
        std::cout << "DebugInspector: Tried to inspect entity " << int(selected) << " but it is invalid.\n";
        return;
    }
        

    std::cout << "Inspecting entity: " << int(selected) << "\n" << "-COMPONENTS-\n";

    if (registry.all_of<Transform>(selected))
    {
        auto& t = registry.get<Transform>(selected);
        std::cout << "  Transform: " << t.matrix.row4.x << ", "
            << t.matrix.row4.y << ", "
            << t.matrix.row4.z << "\n";
    }

    if (registry.all_of<Health>(selected))
    {
        auto& h = registry.get<Health>(selected);
        std::cout << "  Health: " << h.HP << "\n";
    }

    if (registry.all_of<Velocity>(selected))
    {
        auto& v = registry.get<Velocity>(selected);
        std::cout << "  Velocity: " << v.direction.x << ", "
            << v.direction.y << ", "
            << v.direction.z << "\n";
    }

    if (registry.all_of<Shatters>(selected))
    {
        auto& s = registry.get<Shatters>(selected);
        std::cout << " Shatters: \n" <<
            "  Count: " << s.clones << '\n' <<
            "  Remaining: " << s.remaining << '\n' <<
            "  Scale: " << s.scaleDown << '\n';
    }

    if (registry.all_of<Invuln>(selected))
    {
        auto& i = registry.get<Invuln>(selected);
        std::cout << " Invuln Cooldown: " << i.cooldown << '\n';
    }

    // TODO: Show Tags, UI Panel, Component Editing, Mouse Selection
}

void DebugRenderer::DrawLine(const GW::MATH::GVECTORF& a,
    const GW::MATH::GVECTORF& b)
{
    lineVertices.push_back(a);
    lineVertices.push_back(b);
}

void DebugRenderer::Flush()
{
    if (lineVertices.empty())
        return;

    // Upload lineVertices to a dynamic vertex buffer
    // Bind debug line pipeline (simple shader)
    // Issue a draw call: lineVertices.size() / 2 lines

    lineVertices.clear();
}