//
// Created by vector on 24/03/23.
//

#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>

#include "imgui.h"

#include "Logging.h"

#include "Simulation.h"
#include "Gui.h"
#include "Input.h"

GLFWwindow* window;

void initializeWindow() {
  // init GLFW
  if ( !glfwInit() ) {
    mju_error( "Could not initialize GLFW" );
  }

  // create window, make OpenGL context current, request v-sync
  window = glfwCreateWindow( 1200, 900, "Simulate 2.0", NULL, NULL );
  glfwMakeContextCurrent( window );
  glfwSwapInterval( 1 );
}

void processOneFrame() {
  // Process inputs
  Input::process( window, Simulation::model, Simulation::data, &Simulation::opt, &Simulation::pert, &Simulation::scene,
      &Simulation::cam );

  // advance interactive simulation for 1/60 sec
  //  Assuming MuJoCo can simulate faster than real-time, which it usually can,
  //  this loop will finish on time for the next frame to be rendered at 60 fps.
  //  Otherwise add a cpu timer and exit this loop when it is time to render.
  mjtNum simstart = Simulation::data->time;
  while ( Simulation::data->time - simstart < 1.0 / 60.0 ) {
    mj_step( Simulation::model, Simulation::data );
  }

  // get framebuffer viewport
  mjrRect viewport = { 0, 0, 0, 0 };
  glfwGetFramebufferSize( window, &viewport.width, &viewport.height );

  // update scene and render
  DEBUG( "Pert select: {}", Simulation::pert.select );
  mjv_updateScene( Simulation::model, Simulation::data, &Simulation::opt, &Simulation::pert, &Simulation::cam,
      mjCAT_ALL, &Simulation::scene );

  mjr_render( viewport, &Simulation::scene, &Simulation::con );

  Gui::render( Simulation::model, Simulation::data );

  // swap OpenGL buffers (blocking call due to v-sync)
  glfwSwapBuffers( window );

  // process pending GUI events, call GLFW callbacks
  glfwPollEvents();
}


int main() {
  spdlog::set_level( spdlog::level::trace );

  initializeWindow();
  Gui::init( window );

  Simulation::loadScene( "resources/ugv.xml" );

  // run main loop, target real-time simulation and 60 fps rendering
  while ( !glfwWindowShouldClose( window ) ) {
    processOneFrame();
  }

  Simulation::unloadScene();
  Gui::destroy();

  return 0;
}