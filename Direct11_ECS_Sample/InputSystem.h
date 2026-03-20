#pragma once

#include "System.h"
#include <windows.h>
#include <SimpleMath.h>
#include <memory>

class RenderSystem;

class InputSystem : public ISystem
{
public:
    bool Initialize(HWND hwnd);
    void Update(World& world, float deltaTime) override;

    void SetRenderSystem(const std::shared_ptr<RenderSystem>& renderSystem);

    static void ProcessKeyboard(UINT message, WPARAM wParam, LPARAM lParam);
    static void ProcessMouse(UINT message, WPARAM wParam, LPARAM lParam);

private:
    static bool IsKeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

    DirectX::SimpleMath::Vector2 ScreenToWorld(int screenX, int screenY) const;
    POINT GetMouseClientPos() const;

    bool GetKeyMouseLeftButtonTrigger() const  { return  leftDown_ && !prevLeftDown_; }
    bool GetKeyMouseLeftButtonRelease() const  { return !leftDown_ &&  prevLeftDown_; }
    bool GetKeyMouseLeftButtonHeld() const     { return  leftDown_; }
    bool GetKeyMouseRightButtonTrigger() const { return  rightDown_ && !prevRightDown_; }
    bool GetKeyMouseRightButtonRelease() const { return !rightDown_ &&  prevRightDown_; }
    bool GetKeyMouseRightButtonHeld() const    { return  rightDown_; }

    HWND hwnd_ = nullptr;

    bool leftDown_ = false;
    bool rightDown_ = false;
    bool prevLeftDown_ = false;
    bool prevRightDown_ = false;

    int mouseX_ = 0;
    int mouseY_ = 0;

    std::shared_ptr<RenderSystem> renderSystem_;

    DirectX::SimpleMath::Vector2 prevMouseWorld_ = {};
    bool hasPrevMouse_ = false;
};
