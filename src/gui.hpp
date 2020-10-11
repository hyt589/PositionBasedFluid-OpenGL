#pragma once
#include "global.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace ImGui
{
    void BeginGroupPanel(const char *name, const ImVec2 &size = ImVec2(0.0f, 0.0f));
    void EndGroupPanel();
} // namespace ImGui