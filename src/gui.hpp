#pragma once
#include "global.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace ImGui
{
    void BeginGroupPanel(const char *name, const ImVec2 &size = ImVec2(0.0f, 0.0f));
    void EndGroupPanel();
} // namespace ImGui

template <typename ... Args>
void GuiWindow(const char * name, std::function<void(Args ...)> f, Args & ...  params)
{
    ImGui::Begin(name);
    f(params...);
    ImGui::End();
};

template <typename ... Args>
void GuiGroupPanel(const char * name, std::function<void(Args ...)> f, ImVec2 & size = ImVec2(0,0), Args & ... params)
{
    ImGui::BeginGroupPanel(name);
    f(params...);
    ImGui::EndGroupPanel();
};

