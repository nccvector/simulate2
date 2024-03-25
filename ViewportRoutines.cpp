//
// Created by vector on 24/03/24.
//

#include <iostream>
#include "imgui.h"

#include "Input.h"


void _processViewportRotateInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  glfwGetCursorPos( window, &vrMousePosX, &vrMousePosY );
  glfwGetWindowSize( window, &vrWindowWidth, &vrWindowHeight );

  ImGuiIO& io = ImGui::GetIO();

  int leftButtonState = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT );
  if (leftButtonState == GLFW_PRESS) {
    std::cout << "x: " << vrMouseDeltaX << "\ty: " << vrMouseDeltaY << "\n";
    mjv_moveCamera(
        model,
        mjMOUSE_ROTATE_V,
        vrMouseDeltaXNorm,
        vrMouseDeltaYNorm,
        scene,
        camera
    );
  }
}

void _processViewportZoomInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  if (vrMouseScrollDeltaX != 0 || vrMouseScrollDeltaY != 0) {
    std::cout << "scroll x: " << vrMouseScrollDeltaX << "\tscroll y: " << vrMouseScrollDeltaY << "\n";
    mjv_moveCamera(
        model,
        mjMOUSE_ZOOM,
        vrMouseScrollDeltaX * vrZoomSpeed,
        vrMouseScrollDeltaY * vrZoomSpeed,
        scene,
        camera
    );
  }
}

void _processViewportPanInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {

  int middleMouseButtonState = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_MIDDLE );

  if (middleMouseButtonState == GLFW_PRESS) {
    mjv_moveCamera(
        model,
        mjMOUSE_MOVE_V,
        vrMouseDeltaXNorm,
        vrMouseDeltaYNorm,
        scene,
        camera
    );
  }
}

void processViewportInputs( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  // Initialize data for first frame
  _ensureDeltaZeroFirstFrame( window );

  _processViewportRotateInput( window, model, scene, camera );
  _processViewportZoomInput( window, model, scene, camera );
  _processViewportPanInput( window, model, scene, camera );

  // Update deltas
  vrMouseDeltaX     = vrMousePosX - (double) vrMousePreviousX;
  vrMouseDeltaY     = vrMousePosY - (double) vrMousePreviousY;
  vrMouseDeltaXNorm = vrMouseDeltaX / vrWindowWidth;
  vrMouseDeltaYNorm = vrMouseDeltaY / vrWindowHeight;
  // Reset scroll deltas
  vrMouseScrollDeltaX = 0;
  vrMouseScrollDeltaY = 0;

  vrMousePreviousX = vrMousePosX;
  vrMousePreviousY = vrMousePosY;
}