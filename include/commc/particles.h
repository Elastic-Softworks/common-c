/*
   ===================================
   C O M M O N - C
   PARTICLE SYSTEM MODULE
   CG MOON / ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- PARTICLES MODULE ---

    this module provides a minimalist, cross-platform
    abstraction layer for managing and rendering particle systems.
    it defines common structures for particles and emitters,
    and functions for updating and rendering them.

    the actual particle simulation and rendering logic is
    left to the user, allowing integration with various
    graphics APIs or custom rendering techniques, while
    maintaining C89 compliance.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_PARTICLES_H
#define COMMC_PARTICLES_H

#include <stddef.h> /* for size_t */
#include "commc/graphics.h" /* for commc_color_t, commc_rect_t */
#include "commc/cmath.h"     /* for vector operations (conceptual) */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* a single particle. */

typedef struct {

  commc_vec3_t position;     /* current position (x, y, z) */
  commc_vec3_t velocity;     /* current velocity (vx, vy, vz) */
  commc_vec3_t acceleration; /* current acceleration (ax, ay, az) */
  commc_color_t color;       /* current color */
  float        size;         /* current size */
  float        lifetime;     /* remaining lifetime in seconds */
  float        initial_lifetime; /* total lifetime */
  int          active;       /* is this particle active? */

} commc_particle_t;

/* a particle emitter. */

typedef struct commc_particle_emitter_t {

  commc_particle_t* particles;      /* array of particles */
  size_t            capacity;       /* max number of particles */
  size_t            count;          /* current number of active particles */
  commc_vec3_t      position;       /* emitter position */
  float             emission_rate;  /* particles per second */
  float             time_since_emission; /* time since last particle emission */
  commc_vec3_t      velocity_min;   /* min initial velocity */
  commc_vec3_t      velocity_max;   /* max initial velocity */
  float             lifetime_min;   /* min particle lifetime */
  float             lifetime_max;   /* max particle lifetime */
  float             size_min;       /* min particle size */
  float             size_max;       /* max particle size */
  commc_color_t     color_start;    /* starting color */
  commc_color_t     color_end;      /* ending color */
  /* other properties like gravity, drag, etc. could be added */

} commc_particle_emitter_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_particles_init()
	       ---
	       initializes the particle system module.
	       returns 1 on success, 0 on failure.

*/

int commc_particles_init(void);

/*

         commc_particles_shutdown()
	       ---
	       shuts down the particle system module.

*/

void commc_particles_shutdown(void);

/*

         commc_particle_emitter_create()
	       ---
	       creates a new particle emitter with specified properties.
	       returns a pointer to the emitter on success, NULL on failure.
	       the caller is responsible for destroying the emitter.

*/

commc_particle_emitter_t* commc_particle_emitter_create(
  size_t            capacity,
  commc_vec3_t      position,
  float             emission_rate,
  commc_vec3_t      velocity_min,
  commc_vec3_t      velocity_max,
  float             lifetime_min,
  float             lifetime_max,
  float             size_min,
  float             size_max,
  commc_color_t     color_start,
  commc_color_t     color_end
);

/*

         commc_particle_emitter_destroy()
	       ---
	       destroys a particle emitter and frees its resources.

*/

void commc_particle_emitter_destroy(commc_particle_emitter_t* emitter);

/*

         commc_particle_emitter_update()
	       ---
	       updates all active particles in the emitter based on
	       their velocity, lifetime, and emitter properties.
	       'dt' is the time delta in seconds.

*/

void commc_particle_emitter_update(commc_particle_emitter_t* emitter, float dt);

/*

         commc_particle_emitter_render()
	       ---
	       renders all active particles in the emitter.
	       'context' is the graphics context.
	       'camera_pos' is the camera's position for 2D/3D projection.

*/

void commc_particle_emitter_render(commc_particle_emitter_t* emitter, commc_graphics_context_t* context, commc_vec3_t camera_pos);

/*

         commc_particle_emitter_add_particle()
	       ---
	       adds a single particle to the emitter.
	       this is typically called internally by the update logic
	       based on emission rate, but can be used manually.

*/

void commc_particle_emitter_add_particle(commc_particle_emitter_t* emitter);

#endif /* COMMC_PARTICLES_H */

/*
	==================================
             --- EOF ---
	==================================
*/
