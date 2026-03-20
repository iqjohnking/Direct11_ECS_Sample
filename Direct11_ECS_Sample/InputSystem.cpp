#include "InputSystem.h"
#include "RenderSystem.h"
#include "World.h"
#include "TransformComponent.h"
#include "RigidBodyComponent.h"
#include "GrappleComponent.h"
#include "BoxColliderComponent.h"
#include "DragComponent.h"

#include <cmath>
#include <algorithm>

bool InputSystem::Initialize(HWND hwnd)
{
    hwnd_ = hwnd;
    return true;
}

void InputSystem::SetRenderSystem(const std::shared_ptr<RenderSystem>& renderSystem)
{
    renderSystem_ = renderSystem;
}

void InputSystem::ProcessKeyboard(UINT, WPARAM, LPARAM) {}
void InputSystem::ProcessMouse(UINT, WPARAM, LPARAM) {}

POINT InputSystem::GetMouseClientPos() const
{
    POINT pt = {};
    GetCursorPos(&pt);
    if (hwnd_) ScreenToClient(hwnd_, &pt);
    return pt;
}

DirectX::SimpleMath::Vector2 InputSystem::ScreenToWorld(int screenX, int screenY) const
{
    if (!renderSystem_)
    {
        return { static_cast<float>(screenX), static_cast<float>(screenY) };
    }

    // Use actual client area size (correct for fullscreen / resize)
    float w, h;
    if (hwnd_)
    {
        RECT rc;
        GetClientRect(hwnd_, &rc);
        w = static_cast<float>(rc.right - rc.left);
        h = static_cast<float>(rc.bottom - rc.top);
    }
    else
    {
        w = static_cast<float>(renderSystem_->GetScreenWidth());
        h = static_cast<float>(renderSystem_->GetScreenHeight());
    }

    if (w < 1.0f || h < 1.0f)
    {
        return { static_cast<float>(screenX), static_cast<float>(screenY) };
    }

    DirectX::XMMATRIX view = renderSystem_->GetViewMatrix();
    DirectX::XMMATRIX proj = renderSystem_->GetProjectionMatrix();
    DirectX::XMMATRIX vp = view * proj;

    DirectX::XMMATRIX invVP = DirectX::XMMatrixInverse(nullptr, vp);

    float ndcX = (2.0f * screenX / w) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / h);

    DirectX::XMVECTOR ndcPos = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    DirectX::XMVECTOR worldPos = DirectX::XMVector4Transform(ndcPos, invVP);

    float wComp = DirectX::XMVectorGetW(worldPos);
    if (fabsf(wComp) > 1e-8f)
    {
        worldPos = DirectX::XMVectorScale(worldPos, 1.0f / wComp);
    }

    return { DirectX::XMVectorGetX(worldPos), DirectX::XMVectorGetY(worldPos) };
}

static bool RayIntersectsAABB(
    const DirectX::SimpleMath::Vector2& origin,
    const DirectX::SimpleMath::Vector2& dir,
    const DirectX::SimpleMath::Vector2& boxCenter,
    float boxW, float boxH,
    float& outDist,
    DirectX::SimpleMath::Vector2& outPoint)
{
    float halfW = boxW * 0.5f;
    float halfH = boxH * 0.5f;
    float minX = boxCenter.x - halfW;
    float maxX = boxCenter.x + halfW;
    float minY = boxCenter.y - halfH;
    float maxY = boxCenter.y + halfH;

    float tmin = -1e30f;
    float tmax = 1e30f;

    if (fabsf(dir.x) > 1e-8f)
    {
        float t1 = (minX - origin.x) / dir.x;
        float t2 = (maxX - origin.x) / dir.x;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        tmin = (std::max)(tmin, t1);
        tmax = (std::min)(tmax, t2);
    }
    else
    {
        if (origin.x < minX || origin.x > maxX) return false;
    }

    if (fabsf(dir.y) > 1e-8f)
    {
        float t1 = (minY - origin.y) / dir.y;
        float t2 = (maxY - origin.y) / dir.y;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        tmin = (std::max)(tmin, t1);
        tmax = (std::min)(tmax, t2);
    }
    else
    {
        if (origin.y < minY || origin.y > maxY) return false;
    }

    if (tmax < 0.0f || tmin > tmax) return false;

    outDist = (tmin >= 0.0f) ? tmin : tmax;
    if (outDist < 0.0f) return false;

    outPoint = origin + dir * outDist;
    return true;
}

void InputSystem::Update(World& world, float deltaTime)
{
    prevLeftDown_  = leftDown_;
    prevRightDown_ = rightDown_;
    leftDown_  = IsKeyDown(VK_LBUTTON);
    rightDown_ = IsKeyDown(VK_RBUTTON);

    POINT pt = GetMouseClientPos();
    mouseX_ = pt.x;
    mouseY_ = pt.y;

    // Capture edge flags exactly once — these are valid for this frame only
    const bool leftReleasedThisFrame  = GetKeyMouseLeftButtonRelease();
    const bool leftTriggeredThisFrame = GetKeyMouseLeftButtonTrigger();
    const bool rightReleasedThisFrame  = GetKeyMouseRightButtonRelease();
    const bool rightTriggeredThisFrame = GetKeyMouseRightButtonTrigger();

    auto& transforms = world.GetStore<TransformComponent>();
    auto& grapples = world.GetStore<GrappleComponent>();
    auto& colliders = world.GetStore<BoxColliderComponent>();
    auto& drags = world.GetStore<DragComponent>();

    // ========================================================
    // STEP 1: LEFT BUTTON RELEASE — absolute priority, before
    //         any coordinate calculation or physics.
    // ========================================================
    bool shouldDetach = leftReleasedThisFrame;

    // Fallback: button is physically up but grapple still lives
    if (!shouldDetach && !leftDown_)
    {
        for (auto& [entity, grapple] : grapples.All())
        {
            if (grapple.isActive || grapple.isShooting)
            {
                shouldDetach = true;
                break;
            }
        }
    }

    if (shouldDetach)
    {
        for (auto& [entity, grapple] : grapples.All())
        {
            if (grapple.isActive || grapple.isShooting)
            {
                grapple.isActive = false;
                grapple.isShooting = false;
                grapple.didHit = false;
                grapple.currentExtent = 0.0f;
            }
        }

        // Force-reset drag state tied to grapple
        for (auto& [entity, drag] : drags.All())
        {
            if (drag.isDragging)
            {
                drag.isDragging = false;
                drag.shouldLaunch = false;
                drag.launchImpulse = {};
            }
            drag.boostFlashTimer = 0.0f;
            drag.mouseDelta = {};
        }

        // Prevent stale delta on next shot
        hasPrevMouse_ = false;

    }

    // ========================================================
    // STEP 2: Mouse delta calculation (only when relevant)
    // ========================================================
    DirectX::SimpleMath::Vector2 currentMouseWorld = ScreenToWorld(mouseX_, mouseY_);
    DirectX::SimpleMath::Vector2 mouseDelta = {};
    if (hasPrevMouse_)
    {
        mouseDelta = currentMouseWorld - prevMouseWorld_;

        constexpr float deadZone = 1.5f;
        if (mouseDelta.Length() < deadZone)
        {
            mouseDelta = {};
        }
    }
    prevMouseWorld_ = currentMouseWorld;
    hasPrevMouse_ = true;

    // ========================================================
    // STEP 3: Left click — start shooting (only if button held)
    // ========================================================
    for (auto& [entity, grapple] : grapples.All())
    {
        auto* transform = transforms.TryGet(entity);
        if (!transform) continue;

        using namespace DirectX::SimpleMath;
        Vector2 pos(transform->position.x, transform->position.y);

        if (leftTriggeredThisFrame)
        {
            if (!grapple.isActive && !grapple.isShooting)
            {
                Vector2 mouseWorld = ScreenToWorld(mouseX_, mouseY_);
                Vector2 rayDir = mouseWorld - pos;
                float len = rayDir.Length();
                if (len < 1e-4f) continue;
                rayDir /= len;

                grapple.isShooting = true;
                grapple.didHit = false;
                grapple.shootTimer = 0.0f;
                grapple.currentExtent = 0.0f;
                grapple.shootOrigin = pos;
                grapple.shootDir = rayDir;
                grapple.hitDistance = 0.0f;

                float bestDist = 1e30f;
                Vector2 bestPoint;
                bool hit = false;

                for (auto& [blockEntity, collider] : colliders.All())
                {
                    if (!collider.isStatic) continue;

                    auto* blockTransform = transforms.TryGet(blockEntity);
                    if (!blockTransform) continue;

                    Vector2 blockPos(blockTransform->position.x, blockTransform->position.y);
                    float dist = 0.0f;
                    Vector2 point;
                    if (RayIntersectsAABB(pos, rayDir, blockPos,
                        collider.width, collider.height, dist, point))
                    {
                        if (dist < bestDist && dist <= grapple.maxDistance)
                        {
                            bestDist = dist;
                            bestPoint = point;
                            hit = true;
                        }
                    }
                }

                if (hit)
                {
                    grapple.didHit = true;
                    grapple.hitPoint = bestPoint;
                    grapple.hitDistance = bestDist;
                }
            }
        }

        // Shooting animation update
        if (grapple.isShooting)
        {
            grapple.currentExtent += grapple.shootSpeed * deltaTime;

            if (grapple.didHit)
            {
                if (grapple.currentExtent >= grapple.hitDistance)
                {
                    grapple.isShooting = false;
                    grapple.isActive = true;
                    grapple.anchor = grapple.hitPoint;
                    grapple.ropeLength = Vector2::Distance(pos, grapple.hitPoint);
                    grapple.currentExtent = grapple.hitDistance;
                }
            }
            else
            {
                if (grapple.currentExtent >= grapple.maxDistance)
                {
                    grapple.currentExtent = grapple.maxDistance;
                    grapple.isShooting = false;
                }
            }
        }
    }

    // ========================================================
    // STEP 4: Right-click drag (slingshot) — only while grappling
    // ========================================================
    for (auto& [entity, drag] : drags.All())
    {
        auto* transform = transforms.TryGet(entity);
        if (!transform) continue;

        drag.mouseDelta = mouseDelta;

        if (drag.boostFlashTimer > 0.0f)
        {
            drag.boostFlashTimer -= deltaTime;
            if (drag.boostFlashTimer < 0.0f) drag.boostFlashTimer = 0.0f;
        }

        auto* grapple = grapples.TryGet(entity);
        bool isSwinging = grapple && grapple->isActive;

        // Cancel drag if grapple was released during drag
        if (drag.isDragging && !isSwinging)
        {
            drag.isDragging = false;
            drag.shouldLaunch = false;
            drag.launchImpulse = DirectX::SimpleMath::Vector2::Zero;
            drag.boostFlashTimer = 0.0f;
            drag.mouseDelta = {};
            continue;
        }

        using namespace DirectX::SimpleMath;
        Vector2 pos(transform->position.x, transform->position.y);

        if (rightTriggeredThisFrame)
        {
            if (isSwinging)
            {
                drag.isDragging = true;
                drag.shouldLaunch = false;
                drag.dragStartWorld = ScreenToWorld(mouseX_, mouseY_);
                drag.dragCurrentWorld = drag.dragStartWorld;
                drag.launchImpulse = Vector2::Zero;
            }
        }

        if (drag.isDragging && GetKeyMouseRightButtonHeld())
        {
            drag.dragCurrentWorld = ScreenToWorld(mouseX_, mouseY_);

            Vector2 dragVec = drag.dragCurrentWorld - drag.dragStartWorld;
            float dragLen = dragVec.Length();
            float clampedLen = (std::min)(dragLen, drag.maxDragLength);

            if (clampedLen > 1e-4f)
            {
                Vector2 dragDir = dragVec / dragLen;
                drag.launchImpulse = -dragDir * clampedLen * drag.impulseStrength;
            }
            else
            {
                drag.launchImpulse = Vector2::Zero;
            }
        }

        if (rightReleasedThisFrame)
        {
            if (drag.isDragging)
            {
                drag.shouldLaunch = true;
                drag.isDragging = false;
            }
        }
    }
}
