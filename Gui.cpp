//
// Created by vector on 24/03/25.
//

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "fmt/core.h"

#include "Gui.h"
#include "Input.h"
#include "Simulation.h"


#include <map>
#include <iostream>
#include <thread>
#include <chrono>
#include <glob.h>   // glob(), globfree()
#include <string.h> // memset()
#include <vector>
#include <string>

std::vector<std::string> glob( const std::string& pattern ) {
  using namespace std;

  // glob struct resides on the stack
  glob_t glob_result;
  memset( &glob_result, 0, sizeof( glob_result ) );

  // do the glob operation
  int return_value = glob( pattern.c_str(), GLOB_TILDE, NULL, &glob_result );
  if ( return_value != 0 ) {
    globfree( &glob_result );
    return std::vector<string>();
  }

  // collect all the filenames into a std::list<std::string>
  vector<string> filenames;
  for ( size_t i = 0; i < glob_result.gl_pathc; ++i ) {
    filenames.push_back( string( glob_result.gl_pathv[i] ) );
  }

  // cleanup
  globfree( &glob_result );

  // done
  return filenames;
}

std::vector<std::string> getMujocoMenagerieScenePaths() {
  std::vector<std::string> paths = glob( "resources/mujoco_menagerie/*/" );

  std::vector<std::string> sceneXmlPaths;
  for ( auto path : paths ) {
    std::vector<std::string> subSceneXmlPaths = glob( path + "scene*.xml" );

    if ( subSceneXmlPaths.size() > 0 ) {
      // Only take the first scene
      sceneXmlPaths.push_back( subSceneXmlPaths[0] );
    }
  }

  return sceneXmlPaths;
}

namespace Gui {

const float applicationFontSize = 18;
ImFont* font;


void _loadFonts();
void _applyDarkTheme();
void _configureAndSubmitDockspace();
void _createMenuBar( mjvScene* scene, mjvOption* option );
void _createStatePanel( mjModel* model, mjData* data );
void _createControlPanel( mjModel* model, mjData* data );
void _createSceneDropdown() {

  ImGui::Begin( "Select scene" );

  // Only load once
  static std::vector<std::string> paths = getMujocoMenagerieScenePaths();

  const char* items[paths.size()];
  for ( int i = 0; i < paths.size(); i++ ) {
    items[i] = paths[i].c_str();
  }
  static int currentItem = 0;

  ImGui::PushItemWidth( -1 );
  ImGui::ListBox( "Select scene", &currentItem, items, paths.size(), 10 );
  ImGui::PopItemWidth();

  if ( ImGui::Button( "Load", { -1, 30 } ) ) {
    std::cout << "Destroying current scene...\n";
    Simulation::unloadScene(); // TODO: pass this function as a pointer (attachCallback) to get rid of dependency

    std::cout << "Loading " << items[currentItem] << "\n";
    Simulation::loadScene(
        items[currentItem] ); // TODO: pass this function as a pointer (attachCallback) to get rid of dependency
  }

  ImGui::End();
}


void render( mjModel* model, mjData* data ) {
  ImGui::NewFrame();

  // Set theme
  _applyDarkTheme();

  ImGui::PushFont( font );

  // Will allow windows to be docked
  _configureAndSubmitDockspace();

  _createMenuBar( &Simulation::scene, &Simulation::opt ); // TODO: get this scene as a parameter to this function
  _createStatePanel( model, data );
  _createControlPanel( model, data );

  // IMPORTANT: Keep this below (putting it above state and control panel causes segfault)
  // The qpos and ctrl are not fully initialized after loading, they need atleast one updatesim call to initialize
  // A correct fix would be to call updatesim after loading new scene
  _createSceneDropdown();

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
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
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

    ImGui::DragFloat( fmt::format( "ctrl[{0}] ", i ).c_str(), &controlValues[i], 0.01f, -2.0f, 2.0f, "%.3f" );

    // Apply modified control
    data->ctrl[i] = controlValues[i];
  }

  ImGui::End();
}

void __selectableOptionFromFlag( const char* name, mjtByte& flag ) {
  static std::map<std::string, bool> selectStates;

  selectStates[name] = flag;

  ImGui::PushStyleColor(ImGuiCol_Header, {0, 0, 0, 0});
  if ( ImGui::Selectable( name, &selectStates[name] ) ) {
    flag = selectStates[name];
  }
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::Text( "\t\t\t\t" );
  ImGui::SameLine( ImGui::GetWindowWidth() - 30 );
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
  ImGui::PushStyleColor(ImGuiCol_FrameBg, {0, 0, 0, 0});
  ImGui::Checkbox("##", &selectStates[name]);
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
}

void _createMenuBar( mjvScene* scene, mjvOption* option ) {
  if ( ImGui::BeginMainMenuBar() ) {
    if ( ImGui::BeginMenu( "File" ) ) {
      ImGui::MenuItem( "Do nothing..." );
      ImGui::EndMenu();
    }

    if ( ImGui::BeginMenu( "Rendering" ) ) {
      __selectableOptionFromFlag( "Wireframe", scene->flags[mjRND_WIREFRAME] );
      __selectableOptionFromFlag( "Shadows", scene->flags[mjRND_SHADOW] );
      __selectableOptionFromFlag( "Reflection", scene->flags[mjRND_REFLECTION] );
      __selectableOptionFromFlag( "Segmentation", scene->flags[mjRND_SEGMENT] );
      __selectableOptionFromFlag( "Skybox", scene->flags[mjRND_SKYBOX] );

      if ( ImGui::MenuItem( "Disabled option", "[No shortcut]", false, false ) ) {
      } // Disabled item
      ImGui::Separator();
      if ( ImGui::MenuItem( "Do nothing", "CTRL+X" ) ) {
      }

      ImGui::EndMenu();
    }

    if ( ImGui::BeginMenu( "Visualization" ) ) {
      __selectableOptionFromFlag( "Actuator", option->flags[mjVIS_ACTUATOR] );
      __selectableOptionFromFlag( "Joint", option->flags[mjVIS_JOINT] );
      __selectableOptionFromFlag( "Contact Point", option->flags[mjVIS_CONTACTPOINT] );
      __selectableOptionFromFlag( "Contact Force", option->flags[mjVIS_CONTACTFORCE] );
      __selectableOptionFromFlag( "Constraints", option->flags[mjVIS_CONSTRAINT] );
      __selectableOptionFromFlag( "Center of Mass", option->flags[mjVIS_COM] );
      __selectableOptionFromFlag( "Inertia", option->flags[mjVIS_INERTIA] );
      __selectableOptionFromFlag( "Scaled Inertia Boxes", option->flags[mjVIS_SCLINERTIA] );
      __selectableOptionFromFlag( "Perturbation Force", option->flags[mjVIS_PERTFORCE] );
      __selectableOptionFromFlag( "Body BVH", option->flags[mjVIS_BODYBVH] );
      __selectableOptionFromFlag( "Mesh BVH", option->flags[mjVIS_MESHBVH] );

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}


void _applyDarkTheme() {
  ImColor text         = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
  ImColor textDisabled = ImVec4( 1.0f, 1.0f, 1.0f, 0.5f );

  //  // Blue scent
  //  ImColor scent       = ImVec4( 42.0f / 255.0f, 0.0f / 255.0f, 164.0f / 255.0f, 1.0f );
  //  ImColor scentActive       = ImVec4( 0.0f / 255.0f, 163.0f / 255.0f, 255.0f / 255.0f, 1.0f );
  //  ImColor secondScent = scent;

  // Warm scent
  ImColor scent       = ImVec4( 191.0f / 255.0f, 31.0f / 255.0f, 31.0f / 255.0f, 1.0f );
  ImColor scentActive = ImVec4( 217.0f / 255.0f, 83.0f / 255.0f, 35.0f / 255.0f, 1.0f );
  ImColor secondScent = scent;

  ImColor bg     = ImVec4( 0.10f, 0.10f, 0.10f, 1.00f );
  ImColor bgDark = ImVec4( 0.0f, 0.0f, 0.0f, 1.00f );
  ImColor hover  = scentActive;

  ImColor border = ImVec4( 0.1f, 0.1f, 0.1f, 1.0f );
  ImColor shadow = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );

  ImVec4* colors                         = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text]                  = text;
  colors[ImGuiCol_TextDisabled]          = textDisabled;
  colors[ImGuiCol_WindowBg]              = bg;
  colors[ImGuiCol_ChildBg]               = bgDark;
  colors[ImGuiCol_PopupBg]               = bg;
  colors[ImGuiCol_Border]                = border;
  colors[ImGuiCol_BorderShadow]          = shadow;
  colors[ImGuiCol_FrameBg]               = bg;
  colors[ImGuiCol_FrameBgHovered]        = hover;
  colors[ImGuiCol_FrameBgActive]         = scent;
  colors[ImGuiCol_TitleBg]               = bgDark;
  colors[ImGuiCol_TitleBgActive]         = scent;
  colors[ImGuiCol_TitleBgCollapsed]      = scent;
  colors[ImGuiCol_MenuBarBg]             = bg;
  colors[ImGuiCol_ScrollbarBg]           = bgDark;
  colors[ImGuiCol_ScrollbarGrab]         = scent;
  colors[ImGuiCol_ScrollbarGrabHovered]  = hover;
  colors[ImGuiCol_ScrollbarGrabActive]   = scentActive;
  colors[ImGuiCol_CheckMark]             = scent;
  colors[ImGuiCol_SliderGrab]            = scent;
  colors[ImGuiCol_SliderGrabActive]      = scentActive;
  colors[ImGuiCol_Button]                = scent;
  colors[ImGuiCol_ButtonHovered]         = hover;
  colors[ImGuiCol_ButtonActive]          = scentActive;
  colors[ImGuiCol_Header]                = secondScent;
  colors[ImGuiCol_HeaderHovered]         = hover;
  colors[ImGuiCol_HeaderActive]          = scent;
  colors[ImGuiCol_Separator]             = scent;
  colors[ImGuiCol_SeparatorHovered]      = hover;
  colors[ImGuiCol_SeparatorActive]       = scent;
  colors[ImGuiCol_ResizeGrip]            = scent;
  colors[ImGuiCol_ResizeGripHovered]     = hover;
  colors[ImGuiCol_ResizeGripActive]      = scentActive;
  colors[ImGuiCol_Tab]                   = scent;
  colors[ImGuiCol_TabHovered]            = hover;
  colors[ImGuiCol_TabActive]             = scent;
  colors[ImGuiCol_TabUnfocused]          = secondScent;
  colors[ImGuiCol_TabUnfocusedActive]    = scent;
  colors[ImGuiCol_DockingPreview]        = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
  colors[ImGuiCol_DockingEmptyBg]        = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_PlotLines]             = scent;
  colors[ImGuiCol_PlotLinesHovered]      = hover;
  colors[ImGuiCol_PlotHistogram]         = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
  colors[ImGuiCol_PlotHistogramHovered]  = hover;
  colors[ImGuiCol_TableHeaderBg]         = bg;
  colors[ImGuiCol_TableBorderStrong]     = border;
  colors[ImGuiCol_TableBorderLight]      = shadow;
  colors[ImGuiCol_TableRowBg]            = bg;
  colors[ImGuiCol_TableRowBgAlt]         = bgDark;
  colors[ImGuiCol_TextSelectedBg]        = hover;
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
  style.TabBorderSize     = 0;
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
