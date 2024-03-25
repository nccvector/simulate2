//
// Created by vector on 24/03/25.
//
#include "imgui.h"

#include "Input.h"

namespace Input {

const double vrZoomSpeed = 0.025;

double vrMousePosX = 0;
double vrMousePosY = 0;
double vrMousePreviousX = 0;
double vrMousePreviousY = 0;
double vrMouseDeltaX = 0;
double vrMouseDeltaY = 0;
double vrMouseDeltaXNorm = 0;
double vrMouseDeltaYNorm = 0;
double vrMouseScrollDeltaX = 0;
double vrMouseScrollDeltaY = 0;

int vrWindowWidth = 0;
int vrWindowHeight = 0;


// GLFW only supports callbacks for scroll. There is no function like glfwGetScroll etc
void _scrollCallback( GLFWwindow* window, double x, double y );
void _processGuiInputs( GLFWwindow* window );
void _processViewportInputs( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera );


void process( GLFWwindow* window, mjModel* model, mjData* data, mjvScene* scene, mjvCamera* camera ) {
  ImGuiIO& io = ImGui::GetIO();
  if ( io.WantCaptureMouse ) {
    _processGuiInputs( window );
  } else {
    _processViewportInputs( window, model, scene, camera );
  }
}

// GLFW only supports callbacks for scroll. There is no function like glfwGetScroll etc
void _scrollCallback( GLFWwindow* window, double x, double y ) {
  vrMouseScrollDeltaX = x;
  vrMouseScrollDeltaY = y;
}

void ensureDeltaZeroFirstFrame( GLFWwindow* window ) {
  static bool firstFrame = true;
  if ( firstFrame ) {
    vrMousePreviousX  = vrMousePosX;
    vrMousePreviousY  = vrMousePosY;
    vrMouseDeltaX     = 0;
    vrMouseDeltaY     = 0;
    vrMouseDeltaXNorm = 0;
    vrMouseDeltaYNorm = 0;

    // Hook scroll update
    glfwSetScrollCallback( window, _scrollCallback );

    firstFrame = false;
  }
}

void _processGuiInputs( GLFWwindow* window ) {
  ensureDeltaZeroFirstFrame( window );

  ImGuiIO& io    = ImGui::GetIO();
  io.MouseWheelH = vrMouseScrollDeltaX * vrZoomSpeed * 10;
  io.MouseWheel  = vrMouseScrollDeltaY * vrZoomSpeed * 10;

  // Reset after consumption
  vrMouseScrollDeltaX = 0;
  vrMouseScrollDeltaY = 0;
}

} // namespace Input
