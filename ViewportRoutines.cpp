//
// Created by vector on 24/03/24.
//

#include "Input.h"
#include "Logging.h"

namespace Input {

void _processViewportSelectInput( GLFWwindow* window, mjModel* model, mjData* data, mjvOption* opt, mjvPerturb* pert,
    mjvScene* scene, mjvCamera* camera ) {
  int leftCtrlState   = glfwGetKey( window, GLFW_KEY_LEFT_CONTROL );
  int leftButtonState = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT );

  // Compute ray from current mouse coord
  int geomId, flexId, skinId;
  mjtNum selectionPoint[3];

  if ( leftCtrlState == GLFW_PRESS && leftButtonState == GLFW_PRESS ) {
    if ( pert->active == 0 ) {
      float yPos       = 1.0f - Input::vrMousePosY / (double) Input::vrWindowHeight;
      int selectedBody = mjv_select(                                    //
          model,                                                        //
          data,                                                         //
          opt,                                                          //
          (float) Input::vrWindowWidth / (float) Input::vrWindowHeight, //
          Input::vrMousePosX / (double) Input::vrWindowWidth,           //
          yPos,                                                         //
          scene,                                                        //
          selectionPoint,                                               //
          &geomId,                                                      //
          &flexId,                                                      //
          &skinId                                                       //
      );

      DEBUG( "Selected body: {}", selectedBody );
      DEBUG( "Selected point: [{}, {}, {}]", selectionPoint[0], selectionPoint[1], selectionPoint[2] );

      if ( selectedBody > 0 ) {
        mjv_initPerturb( model, data, scene, pert );
        pert->active     = 1;
        pert->select     = selectedBody;
        pert->flexselect = flexId;
        pert->skinselect = skinId;
        pert->scale      = 10;

        // compute localpos
        mjtNum tmp[3];
        mju_sub3( tmp, selectionPoint, data->xpos + 3 * pert->select );
        mju_mulMatTVec( pert->localpos, data->xmat + 9 * pert->select, tmp, 3, 3 );

        // Initialize ref pos from selectionpoint
        pert->refselpos[0] = selectionPoint[0];
        pert->refselpos[1] = selectionPoint[1];
        pert->refselpos[2] = selectionPoint[2];

        DEBUG( "Pert ENGAGED" );
      }
    }


  } else {
    pert->active       = 0;
    pert->select       = -1;
    pert->localpos[0]  = 0;
    pert->localpos[1]  = 0;
    pert->localpos[3]  = 0;
    pert->refselpos[0] = 0;
    pert->refselpos[1] = 0;
    pert->refselpos[3] = 0;

    DEBUG( "Pert RELEASED" );
  }

  if ( pert->active ) {
    // Move ref pos
    mjv_movePerturb(                                          //
        model,                                                //
        data,                                                 //
        mjMOUSE_MOVE_V,                                       //
        Input::vrMouseDeltaX / (float) Input::vrWindowWidth,  //
        Input::vrMouseDeltaY / (float) Input::vrWindowHeight, //
        scene,                                                //
        pert                                                  //
    );

    // Apply perturb
    mjv_applyPerturbForce( model, data, pert );

    DEBUG( "Pert APPLYING FORCE" );
  }
}

void _processViewportRotateInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  glfwGetCursorPos( window, &Input::vrMousePosX, &Input::vrMousePosY );
  glfwGetWindowSize( window, &Input::vrWindowWidth, &Input::vrWindowHeight );

  int leftButtonState = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT );
  if ( leftButtonState == GLFW_PRESS ) {
    mjv_moveCamera(               //
        model,                    //
        mjMOUSE_ROTATE_V,         //
        Input::vrMouseDeltaXNorm, //
        Input::vrMouseDeltaYNorm, //
        scene,                    //
        camera                    //
    );
  }
}

void _processViewportZoomInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {
  if ( Input::vrMouseScrollDeltaX != 0 || Input::vrMouseScrollDeltaY != 0 ) {
    mjv_moveCamera(                                      //
        model,                                           //
        mjMOUSE_ZOOM,                                    //
        Input::vrMouseScrollDeltaX * Input::vrZoomSpeed, //
        Input::vrMouseScrollDeltaY * Input::vrZoomSpeed, //
        scene,                                           //
        camera                                           //
    );
  }
}

void _processViewportPanInput( GLFWwindow* window, mjModel* model, mjvScene* scene, mjvCamera* camera ) {

  int middleMouseButtonState = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_MIDDLE );

  if ( middleMouseButtonState == GLFW_PRESS ) {
    mjv_moveCamera(               //
        model,                    //
        mjMOUSE_MOVE_V,           //
        Input::vrMouseDeltaXNorm, //
        Input::vrMouseDeltaYNorm, //
        scene,                    //
        camera                    //
    );
  }
}

void _processViewportInputs( GLFWwindow* window, mjModel* model, mjData* data, mjvOption* opt, mjvPerturb* pert,
    mjvScene* scene, mjvCamera* camera ) {
  // Initialize data for first frame
  Input::ensureDeltaZeroFirstFrame( window );

  _processViewportRotateInput( window, model, scene, camera );
  _processViewportZoomInput( window, model, scene, camera );
  _processViewportPanInput( window, model, scene, camera );

  _processViewportSelectInput( window, model, data, opt, pert, scene, camera );

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
