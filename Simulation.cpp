//
// Created by vector on 24/03/26.
//

#include <mujoco/mujoco.h>

#include "Simulation.h"

namespace Simulation {

mjModel* model;
mjData* data;
mjvCamera cam;  // abstract camera
mjvOption opt;  // visualization options
mjvScene scene; // abstract scene
mjrContext con; // custom GPU context
mjvPerturb pert;

void loadScene( const char* path ) {
  const int errBufferLen = 1000;
  char err[errBufferLen];
  model = mj_loadXML( path, NULL, err, errBufferLen );
  data  = mj_makeData( model );

  // initialize visualization data structures
  mjv_defaultCamera( &cam );
  mjv_defaultOption( &opt );
  mjv_defaultScene( &scene );
  mjr_defaultContext( &con );

  // create scene and context
  mjv_makeScene( model, &scene, 2000 );
  mjr_makeContext( model, &con, mjFONTSCALE_150 );
}

void unloadScene() {
  // free visualization storage
  mjv_freeScene( &scene );
  mjr_freeContext( &con );

  // free MuJoCo model and data
  mj_deleteData( data );
  mj_deleteModel( model );
}

} // namespace Simulation
