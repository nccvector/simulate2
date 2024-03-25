//
// Created by vector on 24/03/24.
//

#include <iostream>
#include "imgui.h"

#include "Input.h"

namespace Input {

void _processViewportRotateInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  glfwGetCursorPos( window, &Input::vrMousePosX, &Input::vrMousePosY );
  glfwGetWindowSize( window, &Input::vrWindowWidth, &Input::vrWindowHeight );

  ImGuiIO& io = ImGui::GetIO();

  int leftButtonState = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT );
  if ( leftButtonState == GLFW_PRESS ) {
    std::cout << "x: " << Input::vrMouseDeltaX << "\ty: " << Input::vrMouseDeltaY << "\n";
    mjv_moveCamera( model, mjMOUSE_ROTATE_V, Input::vrMouseDeltaXNorm, Input::vrMouseDeltaYNorm, scene, camera );
  }
}

void _processViewportZoomInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  if ( Input::vrMouseScrollDeltaX != 0 || Input::vrMouseScrollDeltaY != 0 ) {
    std::cout << "scroll x: " << Input::vrMouseScrollDeltaX << "\tscroll y: " << Input::vrMouseScrollDeltaY << "\n";
    mjv_moveCamera( model, mjMOUSE_ZOOM, Input::vrMouseScrollDeltaX * Input::vrZoomSpeed,
        Input::vrMouseScrollDeltaY * Input::vrZoomSpeed, scene, camera );
  }
}

void _processViewportPanInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {

  int middleMouseButtonState = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_MIDDLE );

  if ( middleMouseButtonState == GLFW_PRESS ) {
    mjv_moveCamera( model, mjMOUSE_MOVE_V, Input::vrMouseDeltaXNorm, Input::vrMouseDeltaYNorm, scene, camera );
  }
}

void _processViewportInputs( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  // Initialize data for first frame
  Input::ensureDeltaZeroFirstFrame( window );

  _processViewportRotateInput( window, model, scene, camera );
  _processViewportZoomInput( window, model, scene, camera );
  _processViewportPanInput( window, model, scene, camera );

  // Update deltas
  Input::vrMouseDeltaX     = Input::vrMousePosX - (double) Input::vrMousePreviousX;
  Input::vrMouseDeltaY     = Input::vrMousePosY - (double) Input::vrMousePreviousY;
  Input::vrMouseDeltaXNorm = Input::vrMouseDeltaX / Input::vrWindowWidth;
  Input::vrMouseDeltaYNorm = Input::vrMouseDeltaY / Input::vrWindowHeight;
  // Reset scroll deltas
  Input::vrMouseScrollDeltaX = 0;
  Input::vrMouseScrollDeltaY = 0;

  Input::vrMousePreviousX = Input::vrMousePosX;
  Input::vrMousePreviousY = Input::vrMousePosY;
}

} // namespace Input
