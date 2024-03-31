//
// Created by vector on 24/03/26.
//

#ifndef SIMULATE2_SIMULATION_H
#define SIMULATE2_SIMULATION_H

namespace Simulation {

extern mjModel* model;
extern mjData* data;
extern mjvCamera cam;  // abstract camera
extern mjvOption opt;  // visualization options
extern mjvScene scene; // abstract scene
extern mjrContext con; // custom GPU context
extern mjvPerturb pert;

void loadScene(const char* path);
void unloadScene();

} // namespace Simulation

#endif // SIMULATE2_SIMULATION_H
