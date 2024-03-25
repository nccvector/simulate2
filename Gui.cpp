//
// Created by vector on 24/03/25.
//

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "fmt/core.h"

#include "Gui.h"
#include "Input.h"

namespace Gui {

const float applicationFontSize = 18;
ImFont* font;


void _loadFonts();
void _applyDarkTheme();
void _configureAndSubmitDockspace();
void _createStatePanel( mjModel* model, mjData* data );
void _createControlPanel( mjModel* model, mjData* data );


void render( mjModel* model, mjData* data ) {
  ImGui::NewFrame();

  // Set theme
  _applyDarkTheme();

  ImGui::PushFont( font );

  // Will allow windows to be docked
  _configureAndSubmitDockspace();

  _createStatePanel( model, data );
  _createControlPanel( model, data );

  ImGui::ShowDemoWindow( nullptr );

  ImGui::PopFont();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
}

void init( GLFWwindow* window ) {
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL( window, true );
  ImGui_ImplOpenGL3_Init( "#version 330" );

  _loadFonts();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  // Enable docking
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void destroy() {
  // Destroy GUI
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void _loadFonts() {
  // Load default font
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();

  // Load custom font
  font = io.Fonts->AddFontFromFileTTF( "resources/fonts/Roboto-Regular.ttf", applicationFontSize );
}

void _configureAndSubmitDockspace() {
  ImGuiIO& io                               = ImGui::GetIO();
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  // Viewport resizing on window size change
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  viewport->Size          = { (float) Input::vrWindowWidth, (float) Input::vrWindowHeight };
  viewport->WorkSize      = { (float) Input::vrWindowWidth, (float) Input::vrWindowHeight };
  viewport->Flags |= ( viewport->Flags & ImGuiViewportFlags_IsMinimized ); // Preserve existing flags

  // Set dockspace window position and size based on viewport
  ImGui::SetNextWindowPos( viewport->WorkPos );
  ImGui::SetNextWindowSize( viewport->WorkSize );
  ImGui::SetNextWindowViewport( viewport->ID );

  // Set dockspace window style (no border, no padding, no rounding yata yata)
  ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
  ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );

  // Set dockspace window flags (menu bar, no back ground etc)
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
  windowFlags |= ImGuiWindowFlags_NoDocking;
  windowFlags |= ImGuiWindowFlags_NoTitleBar;
  windowFlags |= ImGuiWindowFlags_NoCollapse;
  windowFlags |= ImGuiWindowFlags_NoResize;
  windowFlags |= ImGuiWindowFlags_NoMove;
  windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  windowFlags |= ImGuiWindowFlags_NoNavFocus;
  windowFlags |= ImGuiWindowFlags_NoBackground;

  bool p;
  ImGui::Begin( "DockSpace", &p, windowFlags );

  // Submit dockspace inside a window
  ImGuiID dockspace_id = ImGui::GetID( "MyDockSpace" );
  ImGui::DockSpace( dockspace_id, ImVec2( 0.0f, 0.0f ), dockspace_flags );

  ImGui::End();

  ImGui::PopStyleVar( 3 );
}

void _createStatePanel( mjModel* model, mjData* data ) {
  int numDOFs = model->nq;

  ImGui::Begin( "State" );
  for ( int i = 0; i < numDOFs; i++ ) {
    ImGui::Text( fmt::format( "q[{0}]: {1}", i, data->qpos[i] ).c_str() );
  }

  ImGui::End();
}

void _createControlPanel( mjModel* model, mjData* data ) {
  const int numDOFs = model->nu;
  float controlValues[numDOFs];

  ImGui::Begin( "Control" );
  for ( int i = 0; i < numDOFs; i++ ) {
    // Get current control
    controlValues[i] = data->ctrl[i];

    ImGui::SliderFloat( fmt::format( "ctrl[{0}] ", i ).c_str(), &controlValues[i], -10.0f, 10.0f, "%.3f" );

    // Apply modified control
    data->ctrl[i] = controlValues[i];
  }

  ImGui::End();
}


void _applyDarkTheme() {
  ImVec4* colors                         = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text]                  = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
  colors[ImGuiCol_TextDisabled]          = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
  colors[ImGuiCol_WindowBg]              = ImVec4( 0.10f, 0.10f, 0.10f, 1.00f );
  colors[ImGuiCol_ChildBg]               = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
  colors[ImGuiCol_PopupBg]               = ImVec4( 0.19f, 0.19f, 0.19f, 0.92f );
  colors[ImGuiCol_Border]                = ImVec4( 0.19f, 0.19f, 0.19f, 0.29f );
  colors[ImGuiCol_BorderShadow]          = ImVec4( 0.00f, 0.00f, 0.00f, 0.24f );
  colors[ImGuiCol_FrameBg]               = ImVec4( 0.05f, 0.05f, 0.05f, 0.54f );
  colors[ImGuiCol_FrameBgHovered]        = ImVec4( 0.19f, 0.19f, 0.19f, 0.54f );
  colors[ImGuiCol_FrameBgActive]         = ImVec4( 0.20f, 0.22f, 0.23f, 1.00f );
  colors[ImGuiCol_TitleBg]               = ImVec4( 0.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_TitleBgActive]         = ImVec4( 0.06f, 0.06f, 0.06f, 1.00f );
  colors[ImGuiCol_TitleBgCollapsed]      = ImVec4( 0.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_MenuBarBg]             = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
  colors[ImGuiCol_ScrollbarBg]           = ImVec4( 0.05f, 0.05f, 0.05f, 0.54f );
  colors[ImGuiCol_ScrollbarGrab]         = ImVec4( 0.34f, 0.34f, 0.34f, 0.54f );
  colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4( 0.40f, 0.40f, 0.40f, 0.54f );
  colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4( 0.56f, 0.56f, 0.56f, 0.54f );
  colors[ImGuiCol_CheckMark]             = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
  colors[ImGuiCol_SliderGrab]            = ImVec4( 0.34f, 0.34f, 0.34f, 0.54f );
  colors[ImGuiCol_SliderGrabActive]      = ImVec4( 0.56f, 0.56f, 0.56f, 0.54f );
  colors[ImGuiCol_Button]                = ImVec4( 0.05f, 0.05f, 0.05f, 0.54f );
  colors[ImGuiCol_ButtonHovered]         = ImVec4( 0.19f, 0.19f, 0.19f, 0.54f );
  colors[ImGuiCol_ButtonActive]          = ImVec4( 0.20f, 0.22f, 0.23f, 1.00f );
  colors[ImGuiCol_Header]                = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
  colors[ImGuiCol_HeaderHovered]         = ImVec4( 0.00f, 0.00f, 0.00f, 0.36f );
  colors[ImGuiCol_HeaderActive]          = ImVec4( 0.20f, 0.22f, 0.23f, 0.33f );
  colors[ImGuiCol_Separator]             = ImVec4( 0.28f, 0.28f, 0.28f, 0.29f );
  colors[ImGuiCol_SeparatorHovered]      = ImVec4( 0.44f, 0.44f, 0.44f, 0.29f );
  colors[ImGuiCol_SeparatorActive]       = ImVec4( 0.40f, 0.44f, 0.47f, 1.00f );
  colors[ImGuiCol_ResizeGrip]            = ImVec4( 0.28f, 0.28f, 0.28f, 0.29f );
  colors[ImGuiCol_ResizeGripHovered]     = ImVec4( 0.44f, 0.44f, 0.44f, 0.29f );
  colors[ImGuiCol_ResizeGripActive]      = ImVec4( 0.40f, 0.44f, 0.47f, 1.00f );
  colors[ImGuiCol_Tab]                   = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
  colors[ImGuiCol_TabHovered]            = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
  colors[ImGuiCol_TabActive]             = ImVec4( 0.20f, 0.20f, 0.20f, 0.36f );
  colors[ImGuiCol_TabUnfocused]          = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
  colors[ImGuiCol_TabUnfocusedActive]    = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
  colors[ImGuiCol_DockingPreview]        = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
  colors[ImGuiCol_DockingEmptyBg]        = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_PlotLines]             = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_PlotLinesHovered]      = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_PlotHistogram]         = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_PlotHistogramHovered]  = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_TableHeaderBg]         = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
  colors[ImGuiCol_TableBorderStrong]     = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
  colors[ImGuiCol_TableBorderLight]      = ImVec4( 0.28f, 0.28f, 0.28f, 0.29f );
  colors[ImGuiCol_TableRowBg]            = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
  colors[ImGuiCol_TableRowBgAlt]         = ImVec4( 1.00f, 1.00f, 1.00f, 0.06f );
  colors[ImGuiCol_TextSelectedBg]        = ImVec4( 0.20f, 0.22f, 0.23f, 1.00f );
  colors[ImGuiCol_DragDropTarget]        = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
  colors[ImGuiCol_NavHighlight]          = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 1.00f, 0.00f, 0.00f, 0.70f );
  colors[ImGuiCol_NavWindowingDimBg]     = ImVec4( 1.00f, 0.00f, 0.00f, 0.20f );
  colors[ImGuiCol_ModalWindowDimBg]      = ImVec4( 1.00f, 0.00f, 0.00f, 0.35f );

  ImGuiStyle& style       = ImGui::GetStyle();
  style.WindowPadding     = ImVec2( 8.00f, 8.00f );
  style.FramePadding      = ImVec2( 5.00f, 2.00f );
  style.CellPadding       = ImVec2( 6.00f, 6.00f );
  style.ItemSpacing       = ImVec2( 6.00f, 6.00f );
  style.ItemInnerSpacing  = ImVec2( 6.00f, 6.00f );
  style.TouchExtraPadding = ImVec2( 0.00f, 0.00f );
  style.IndentSpacing     = 25;
  style.ScrollbarSize     = 15;
  style.GrabMinSize       = 10;
  style.WindowBorderSize  = 1;
  style.ChildBorderSize   = 1;
  style.PopupBorderSize   = 1;
  style.FrameBorderSize   = 1;
  style.TabBorderSize     = 1;
  style.WindowRounding    = 7;
  style.ChildRounding     = 4;
  style.FrameRounding     = 3;
  style.PopupRounding     = 4;
  style.ScrollbarRounding = 9;
  style.GrabRounding      = 3;
  style.LogSliderDeadzone = 4;
  style.TabRounding       = 1;
}

} // namespace Gui
