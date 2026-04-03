#pragma once

#include <imgui.h>

class Styles {
public:
    static void setupDarkTheme() {
        ImGuiStyle& style = ImGui::GetStyle();

        // Main Sizes
        style.WindowPadding     = ImVec2(8, 8);
        style.FramePadding      = ImVec2(4, 2);
        style.ItemSpacing       = ImVec2(8, 5);
        style.ItemInnerSpacing  = ImVec2(4, 4);
        style.IndentSpacing     = 21.0f;
        style.GrabMinSize       = 12.0f;

        // Borders
        style.WindowBorderSize  = 0.0f;
        style.ChildBorderSize   = 1.0f;
        style.PopupBorderSize   = 1.0f;
        style.FrameBorderSize   = 0.0f;
        style.TabBorderSize     = 0.0f;
        style.TabBarBorderSize  = 1.0f;

        // Rounding
        style.WindowRounding    = 8.0f;
        style.ChildRounding     = 6.0f;
        style.FrameRounding     = 6.0f;
        style.PopupRounding     = 6.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 7.0f;
        style.ScrollbarRounding = 9.0f;

        // Tables & Widgets
        style.CellPadding       = ImVec2(4, 2);
        style.SeparatorSize     = 1.0f;
        style.LogSliderDeadzone = 4.0f;

        // Alignment
        style.WindowTitleAlign  = ImVec2(0.0f, 0.5f);
        style.ButtonTextAlign   = ImVec2(0.5f, 0.5f);

        style.DockingSeparatorSize = 0.0f;

        style.WindowMenuButtonPosition = ImGuiDir_None;

        // COLORS
        ImVec4* colors = style.Colors;
        
        colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.07f, 0.07f, 0.07f, 0.98f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.34f, 0.34f, 0.34f, 0.40f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.12f, 0.12f, 0.12f, 0.67f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.41f, 0.41f, 0.41f, 0.40f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.29f, 0.29f, 0.29f, 0.31f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.23f, 0.23f, 0.23f, 0.80f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.42f, 0.42f, 0.42f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.44f, 0.44f, 0.44f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.69f, 0.69f, 0.69f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.76f, 0.76f, 0.76f, 0.95f);
        colors[ImGuiCol_InputTextCursor]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.29f, 0.29f, 0.29f, 0.80f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.18f, 0.18f, 0.86f);
        colors[ImGuiCol_TabSelected]            = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
        colors[ImGuiCol_DockingPreview]         = ImVec4(0.52f, 0.52f, 0.52f, 0.70f);
        colors[ImGuiCol_TabDimmed]              = ImVec4(0.10f, 0.12f, 0.14f, 0.97f);
        colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TabDimmedSelectedOverline]  = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
        colors[ImGuiCol_DockingPreview]         = ImVec4(0.98f, 0.79f, 0.26f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextLink]               = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_TreeLines]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_DragDropTargetBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_UnsavedMarker]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_NavCursor]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        colors[ImGuiCol_TabSelectedOverline]    = ImVec4(0.00f, 0.46f, 1.00f, 0.00f);
    }
};