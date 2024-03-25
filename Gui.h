//
// Created by vector on 24/03/25.
//

#ifndef SIMULATE2_GUI_H
#define SIMULATE2_GUI_H

#endif // SIMULATE2_GUI_H

#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include "imgui.h"

extern const float applicationFontSize;
extern ImFont* font;

void _loadFonts();

void initGui( GLFWwindow* window );
void destroyGui();

void _embraceTheDarkness();

void _configureAndSubmitDockspace();

void _createStatePanel( mjModel* model, mjData* data );

void _createControlPanel( mjModel* model, mjData* data );

void renderGui( mjModel* model, mjData* data );
