//
// Created by vector on 24/03/24.
//

#ifndef VIEWPORTROUTINES_H
#define VIEWPORTROUTINES_H

#endif //VIEWPORTROUTINES_H

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

// GLFW only supports callbacks for scroll. There is no function like glfwGetScroll etc
void _scrollCallback( GLFWwindow* window, double x, double y ) {
  vrMouseScrollDeltaX = x;
  vrMouseScrollDeltaY = y;
}

void _ensureDeltaZeroFirstFrame( GLFWwindow* window ) {
  static bool firstFrame = true;
  if (firstFrame) {
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

void processViewportRotateInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
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

void processViewportZoomInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
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

void processViewportPanInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {

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

  processViewportRotateInput( window, model, scene, camera );
  processViewportZoomInput( window, model, scene, camera );
  processViewportPanInput( window, model, scene, camera );

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