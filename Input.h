//
// Created by vector on 24/03/25.
//

#ifndef SIMULATE2_INPUT_H
#define SIMULATE2_INPUT_H

#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>

namespace Input {

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

void ensureDeltaZeroFirstFrame( GLFWwindow* window );
void process( GLFWwindow* window, mjModel* model, mjData* data, mjvScene* scene, mjvCamera* camera );

} // namespace Input

#endif // SIMULATE2_INPUT_H
