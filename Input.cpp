//
// Created by vector on 24/03/25.
//
#include "imgui.h"

#include "Input.h"


const double vrZoomSpeed = 0.025;

double vrMousePosX;
double vrMousePosY;
double vrMousePreviousX;
double vrMousePreviousY;
double vrMouseDeltaX;
double vrMouseDeltaY;
double vrMouseDeltaXNorm;
double vrMouseDeltaYNorm;
double vrMouseScrollDeltaX;
double vrMouseScrollDeltaY;

int vrWindowWidth;
int vrWindowHeight;

void processInputs( GLFWwindow* window, mjModel* model, mjData* data, mjvScene* scene, mjvCamera* camera ) {
  ImGuiIO& io = ImGui::GetIO();
  if ( io.WantCaptureMouse ) {
    processGuiInputs( window );
  } else {
    processViewportInputs( window, model, scene, camera );
  }
}

// GLFW only supports callbacks for scroll. There is no function like glfwGetScroll etc
void _scrollCallback( GLFWwindow* window, double x, double y ) {
  vrMouseScrollDeltaX = x;
  vrMouseScrollDeltaY = y;
}

void _ensureDeltaZeroFirstFrame( GLFWwindow* window ) {
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

void processGuiInputs( GLFWwindow* window ) {
  _ensureDeltaZeroFirstFrame( window );

  ImGuiIO& io    = ImGui::GetIO();
  io.MouseWheelH = vrMouseScrollDeltaX * vrZoomSpeed * 10;
  io.MouseWheel  = vrMouseScrollDeltaY * vrZoomSpeed * 10;

  // Reset after consumption
  vrMouseScrollDeltaX = 0;
  vrMouseScrollDeltaY = 0;
}