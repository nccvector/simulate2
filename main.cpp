//
// Created by vector on 24/03/23.
//

#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>

#include "imgui.h"

#include "Gui.h"
#include "Input.h"


int main() {
  const int errBufferLen = 1000;
  char err[errBufferLen];
  // mjModel* model = mj_loadXML( "resources/ugv.xml", NULL, err, errBufferLen );
  mjModel* model = mj_loadXML( "resources/mujoco_menagerie/kuka_iiwa_14/scene.xml", NULL, err, errBufferLen );
  mjData* data   = mj_makeData( model );
  mjvCamera cam;  // abstract camera
  mjvOption opt;  // visualization options
  mjvScene scene; // abstract scene
  mjrContext con; // custom GPU context

  // init GLFW
  if ( !glfwInit() ) {
    mju_error( "Could not initialize GLFW" );
  }

  // create window, make OpenGL context current, request v-sync
  GLFWwindow* window = glfwCreateWindow( 1200, 900, "Demo", NULL, NULL );
  glfwMakeContextCurrent( window );
  glfwSwapInterval( 1 );

  Gui::init( window );

  // initialize visualization data structures
  mjv_defaultCamera( &cam );
  mjv_defaultOption( &opt );
  mjv_defaultScene( &scene );
  mjr_defaultContext( &con );

  // create scene and context
  mjv_makeScene( model, &scene, 2000 );
  mjr_makeContext( model, &con, mjFONTSCALE_150 );

  // run main loop, target real-time simulation and 60 fps rendering
  while ( !glfwWindowShouldClose( window ) ) {
    // Process inputs
    Input::process( window, model, data, &scene, &cam );

    // advance interactive simulation for 1/60 sec
    //  Assuming MuJoCo can simulate faster than real-time, which it usually can,
    //  this loop will finish on time for the next frame to be rendered at 60 fps.
    //  Otherwise add a cpu timer and exit this loop when it is time to render.
    mjtNum simstart = data->time;
    while ( data->time - simstart < 1.0 / 60.0 ) {
      mj_step( model, data );
    }

    // get framebuffer viewport
    mjrRect viewport = { 0, 0, 0, 0 };
    glfwGetFramebufferSize( window, &viewport.width, &viewport.height );

    // update scene and render
    mjv_updateScene( model, data, &opt, NULL, &cam, mjCAT_ALL, &scene );

    mjr_render( viewport, &scene, &con );

    Gui::render( model, data );

    // swap OpenGL buffers (blocking call due to v-sync)
    glfwSwapBuffers( window );

    // process pending GUI events, call GLFW callbacks
    glfwPollEvents();
  }

  Gui::destroy();

  // free visualization storage
  mjv_freeScene( &scene );
  mjr_freeContext( &con );

  // free MuJoCo model and data
  mj_deleteData( data );
  mj_deleteModel( model );

  return 0;
}