//
// Created by vector on 24/03/25.
//

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Logging.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/callback_sink.h"

#include "fmt/core.h"

#include "Gui.h"
#include "Input.h"
#include "Simulation.h"
#include "Console.h"


#include <map>
#include <glob.h> // glob(), globfree()
#include <vector>
#include <string>

using namespace ImGui;

const float _applicationFontSize = 18;
const float _consoleFontSize     = 18;
ImFont* _applicationFont;
ImFont* _applicationFontBold;
ImFont* _consoleFont;
ImFont* _consoleFontBold;

Console* _console;

void RegisterLogCallback() {
  auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>( //
      []( const spdlog::details::log_msg& msg ) {
        _console->AddLog( msg.payload.data() );
        // for example you can be notified by sending an email to yourself
      } //
  );

  callback_sink->set_level( spdlog::level::trace );

  //  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  //  spdlog::default_logger()->sinks().push_back(console_sink);
  spdlog::default_logger()->sinks().push_back( callback_sink );
}


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


void _loadFonts();
void _applyDarkTheme();
void _configureAndSubmitDockspace();
void _createMenuBar( mjvScene* scene, mjvOption* option );
void _createStatePanel( mjModel* model, mjData* data );
void _createControlPanel( mjModel* model, mjData* data );
void _createSceneDropdown() {

  Begin( "Select scene" );

  // Only load once
  static std::vector<std::string> paths = getMujocoMenagerieScenePaths();

  const char* items[paths.size()];
  for ( int i = 0; i < paths.size(); i++ ) {
    items[i] = paths[i].c_str();
  }
  static int currentItem = 0;

  PushItemWidth( -1 );
  ListBox( "Select scene", &currentItem, items, paths.size(), 10 );
  PopItemWidth();

  if ( Button( "Load", { -1, 30 } ) ) {
    DEBUG( "Destroying current scene", nullptr );
    Simulation::unloadScene(); // TODO: pass this function as a pointer (attachCallback) to get rid of dependency

    DEBUG( "Loading {}", items[currentItem] );
    Simulation::loadScene(
        items[currentItem] ); // TODO: pass this function as a pointer (attachCallback) to get rid of dependency
  }

  End();
}


void render( mjModel* model, mjData* data ) {
  NewFrame();

  // Set theme
  _applyDarkTheme();

  PushFont( _applicationFont );

  // Will allow windows to be docked
  _configureAndSubmitDockspace();

  _createMenuBar( &Simulation::scene, &Simulation::opt ); // TODO: get this scene as a parameter to this function
  _createStatePanel( model, data );
  _createControlPanel( model, data );

  // IMPORTANT: Keep this below (putting it above state and control panel causes segfault)
  // The qpos and ctrl are not fully initialized after loading, they need atleast one updatesim call to initialize
  // A correct fix would be to call updatesim after loading new scene
  _createSceneDropdown();

  static bool consoleOpen = true;
  _console->Draw( "Console", &consoleOpen );

  ShowDemoWindow( nullptr );

  PopFont();

  Render();
  ImGui_ImplOpenGL3_RenderDrawData( GetDrawData() );
}

void init( GLFWwindow* window ) {

  CreateContext();
  StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL( window, true );
  ImGui_ImplOpenGL3_Init( "#version 330" );

  _loadFonts();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  // Enable docking
  ImGuiIO& io = GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Create console
  _console = new Console();

  // Bind console output to spdlog log callback
  RegisterLogCallback();
}

void destroy() {
  // Destroy GUI
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  DestroyContext();
}

void _loadFonts() {
  // Load default font
  ImGuiIO& io = GetIO();
  io.Fonts->AddFontDefault();

  // Load custom font
  _applicationFont     = io.Fonts->AddFontFromFileTTF( "resources/fonts/Roboto-Regular.ttf", _applicationFontSize );
  _applicationFontBold = io.Fonts->AddFontFromFileTTF( "resources/fonts/Roboto-Medium.ttf", _applicationFontSize );
  _consoleFont         = io.Fonts->AddFontFromFileTTF( "resources/fonts/FiraCode-Regular.ttf", _consoleFontSize );
  _consoleFontBold     = io.Fonts->AddFontFromFileTTF( "resources/fonts/FiraCode-SemiBold.ttf", _consoleFontSize );
}

void _configureAndSubmitDockspace() {
  ImGuiIO& io                               = GetIO();
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  // Viewport resizing on window size change
  ImGuiViewport* viewport = GetMainViewport();
  viewport->Size          = { (float) Input::vrWindowWidth, (float) Input::vrWindowHeight };
  viewport->WorkSize      = { (float) Input::vrWindowWidth, (float) Input::vrWindowHeight };
  viewport->Flags |= ( viewport->Flags & ImGuiViewportFlags_IsMinimized ); // Preserve existing flags

  // Set dockspace window position and size based on viewport
  SetNextWindowPos( viewport->WorkPos );
  SetNextWindowSize( viewport->WorkSize );
  SetNextWindowViewport( viewport->ID );

  // Set dockspace window style (no border, no padding, no rounding yata yata)
  PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
  PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
  PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );

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
  Begin( "DockSpace", &p, windowFlags );

  // Submit dockspace inside a window
  ImGuiID dockspace_id = GetID( "MyDockSpace" );
  DockSpace( dockspace_id, ImVec2( 0.0f, 0.0f ), dockspace_flags );

  End();

  PopStyleVar( 3 );
}

void _createStatePanel( mjModel* model, mjData* data ) {
  int numDOFs = model->nq;

  Begin( "State" );
  for ( int i = 0; i < numDOFs; i++ ) {
    PushFont( _applicationFontBold );
    Text( fmt::format( "q[{0}]: ", i ).c_str() );
    PopFont();

    float windowWidth      = GetWindowWidth();
    float itemWidth        = windowWidth * 0.7;
    float itemPaddingRight = 20;

    SameLine( windowWidth - itemWidth - itemPaddingRight );

    PushItemWidth( itemWidth );

    PushFont( _consoleFont );
    PushStyleColor( ImGuiCol_Text, { 1.0, 1.0, 1.0, 0.7 } );
    PushStyleColor( ImGuiCol_FrameBgHovered, { 0.0, 0.0, 0.0, 1.0 } );
    PushStyleColor( ImGuiCol_FrameBgActive, { 0.0, 0.0, 0.0, 1.0 } );
    float val = data->qpos[i];
    DragFloat( fmt::format( "##q[{0}]: ", i ).c_str(), &val, 0.0000001f, -1000000.0f, 1000000.0f, "%.4f",
        ImGuiSliderFlags_NoInput );
    PopStyleColor( 3 );
    PopFont();

    PopItemWidth();
  }
  End();
}

void _createControlPanel( mjModel* model, mjData* data ) {
  const int numDOFs = model->nu;
  float controlValues[numDOFs];

  Begin( "Control" );
  for ( int i = 0; i < numDOFs; i++ ) {
    // Get current control
    controlValues[i] = data->ctrl[i];

    PushFont( _applicationFontBold );
    Text( fmt::format( "Control[{0}]: ", i ).c_str() );
    PopFont();

    float windowWidth      = GetWindowWidth();
    float itemWidth        = windowWidth * 0.6;
    float itemPaddingRight = 20;

    SameLine( windowWidth - itemWidth - itemPaddingRight );

    PushItemWidth( itemWidth );

    PushFont( _consoleFont );
    DragFloat( fmt::format( "##ctrl[{0}]", i ).c_str(), &controlValues[i], 0.01f, -2.0f, 2.0f, "%.2f" );
    PopFont();

    PopItemWidth();

    // Apply modified control
    data->ctrl[i] = controlValues[i];
  }

  End();
}

void __selectableOptionFromFlag( const char* name, mjtByte& flag ) {
  static std::map<std::string, bool> selectStates;

  selectStates[name] = flag;

  PushStyleColor( ImGuiCol_Header, { 0, 0, 0, 0 } );
  if ( Selectable( name, &selectStates[name] ) ) {
    flag = selectStates[name];
  }
  PopStyleColor();
  SameLine();
  Text( "\t\t\t\t" );
  SameLine( GetWindowWidth() - 30 );
  PushStyleVar( ImGuiStyleVar_FrameBorderSize, 0 );
  PushStyleColor( ImGuiCol_FrameBg, { 0, 0, 0, 0 } );
  Checkbox( "##", &selectStates[name] );
  PopStyleColor();
  PopStyleVar();
}

void _createMenuBar( mjvScene* scene, mjvOption* option ) {
  if ( BeginMainMenuBar() ) {
    if ( BeginMenu( "File" ) ) {
      MenuItem( "Do nothing..." );
      EndMenu();
    }

    if ( BeginMenu( "Rendering" ) ) {
      __selectableOptionFromFlag( "Wireframe", scene->flags[mjRND_WIREFRAME] );
      __selectableOptionFromFlag( "Shadows", scene->flags[mjRND_SHADOW] );
      __selectableOptionFromFlag( "Reflection", scene->flags[mjRND_REFLECTION] );
      __selectableOptionFromFlag( "Segmentation", scene->flags[mjRND_SEGMENT] );
      __selectableOptionFromFlag( "Skybox", scene->flags[mjRND_SKYBOX] );

      if ( MenuItem( "Disabled option", "[No shortcut]", false, false ) ) {
      } // Disabled item
      Separator();
      if ( MenuItem( "Do nothing", "CTRL+X" ) ) {
      }

      EndMenu();
    }

    if ( BeginMenu( "Visualization" ) ) {
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

      EndMenu();
    }

    EndMainMenuBar();
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

  ImVec4* colors                         = GetStyle().Colors;
  colors[ImGuiCol_Text]                  = text;
  colors[ImGuiCol_TextDisabled]          = textDisabled;
  colors[ImGuiCol_WindowBg]              = bg;
  colors[ImGuiCol_ChildBg]               = bgDark;
  colors[ImGuiCol_PopupBg]               = bg;
  colors[ImGuiCol_Border]                = border;
  colors[ImGuiCol_BorderShadow]          = shadow;
  colors[ImGuiCol_FrameBg]               = bgDark;
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

  int scentRounding = 1;
  int noRounding    = 0;
  int maxRounding   = 5;

  ImGuiStyle& style       = GetStyle();
  style.WindowPadding     = ImVec2( 8.00f, 8.00f );
  style.FramePadding      = ImVec2( 5.00f, 2.00f );
  style.CellPadding       = ImVec2( 6.00f, 6.00f );
  style.ItemSpacing       = ImVec2( 6.00f, 6.00f );
  style.ItemInnerSpacing  = ImVec2( 6.00f, 6.00f );
  style.TouchExtraPadding = ImVec2( 0.00f, 0.00f );
  style.IndentSpacing     = 25;
  style.ScrollbarSize     = 5;
  style.GrabMinSize       = 10;
  style.WindowBorderSize  = 0;
  style.ChildBorderSize   = 1;
  style.PopupBorderSize   = 1;
  style.FrameBorderSize   = 0;
  style.TabBorderSize     = 0;
  style.WindowRounding    = maxRounding;
  style.ChildRounding     = maxRounding;
  style.FrameRounding     = scentRounding;
  style.PopupRounding     = noRounding;
  style.ScrollbarRounding = noRounding;
  style.GrabRounding      = scentRounding;
  style.TabRounding       = scentRounding;
  style.LogSliderDeadzone = 4;
}

} // namespace Gui
