//
// Created by vector on 24/03/25.
//

#ifndef SIMULATE2_GUI_H
#define SIMULATE2_GUI_H


#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include "imgui.h"

namespace Gui {

void init( GLFWwindow* window );
void destroy();
void render( mjModel* model, mjData* data );

} // namespace Gui

#endif // SIMULATE2_GUI_H
