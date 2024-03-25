//
// Created by vector on 24/03/25.
//

#ifndef SIMULATE2_INPUT_H
#define SIMULATE2_INPUT_H

#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>

extern const double vrZoomSpeed;

extern double vrMousePosX;
extern double vrMousePosY;
extern double vrMousePreviousX;
extern double vrMousePreviousY;
extern double vrMouseDeltaX;
extern double vrMouseDeltaY;
extern double vrMouseDeltaXNorm;
extern double vrMouseDeltaYNorm;
extern double vrMouseScrollDeltaX;
extern double vrMouseScrollDeltaY;

extern int vrWindowWidth;
extern int vrWindowHeight;

// GLFW only supports callbacks for scroll. There is no function like glfwGetScroll etc
void _scrollCallback( GLFWwindow* window, double x, double y );
void _ensureDeltaZeroFirstFrame( GLFWwindow* window );
void processGuiInputs(GLFWwindow* window);
void processViewportInputs( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera );

#endif // SIMULATE2_INPUT_H
