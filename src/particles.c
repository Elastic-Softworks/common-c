/*
   ===================================
   C O M M O N - C
   PARTICLE SYSTEM IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- PARTICLES MODULE ---

    implementation of the particle system for managing and
    updating collections of particles. this provides the core
    logic for particle emitters, lifetime management, and
    basic particle physics simulation.

    see include/commc/particles.h for function prototypes
    and detailed documentation of the particle system API.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>                 /* for malloc, free, rand */
#include <string.h>                 /* for memset */
#include <math.h>                   /* for mathematical operations */

#include "commc/particles.h"
#include "commc/error.h"            /* for error reporting */
#include "commc/math.h"             /* for vector operations */

/*
	==================================
             --- GLOBALS ---
	==================================
*/

static int particles_initialized = 0;  /* module initialization state */

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         interpolate_color()
	       ---
	       linearly interpolates between two colors based on
	       a time factor. used to animate particle colors
	       from start to end over their lifetime.

	       t should be between 0.0 and 1.0, where 0.0 gives
	       start_color and 1.0 gives end_color.

*/

static commc_color_t interpolate_color(commc_color_t start, commc_color_t end, float t) {

  commc_color_t result;

  /* clamp t to valid range */

  if  (t < 0.0f) t = 0.0f;
  if  (t > 1.0f) t = 1.0f;

  /* interpolate each color component */

  result.r = (unsigned char)(start.r + t * (end.r - start.r));
  result.g = (unsigned char)(start.g + t * (end.g - start.g));
  result.b = (unsigned char)(start.b + t * (end.b - start.b));
  result.a = (unsigned char)(start.a + t * (end.a - start.a));

  return result;

}

/*

         random_float_range()
	    ---
	    generates a random floating-point number between
	    min and max values. used for randomizing particle
	    properties like velocity, lifetime, and size.

*/

static float random_float_range(float min_val, float max_val) {

  float t;

  /* generate random value between 0.0 and 1.0 */

  t = (float)rand() / (float)RAND_MAX;

  /* scale and offset to desired range */

  return min_val + t * (max_val - min_val);

}

/*

         random_vec3_range()
	       ---
	       generates a random 3D vector with each component
	       between the corresponding min and max values.
	       useful for randomizing particle velocities.

*/

static commc_vec3_t random_vec3_range(commc_vec3_t min_vec, commc_vec3_t max_vec) {

  commc_vec3_t result;

  result.x = random_float_range(min_vec.x, max_vec.x);
  result.y = random_float_range(min_vec.y, max_vec.y);
  result.z = random_float_range(min_vec.z, max_vec.z);

  return result;

}

/*

         find_inactive_particle()
	       ---
	       searches through the emitter's particle array to
	       find an inactive particle that can be reused.
	       returns the index of the first inactive particle,
	       or -1 if all particles are active.

*/

static int find_inactive_particle(commc_particle_emitter_t* emitter) {

  size_t i;

  for  (i = 0; i < emitter->capacity; i++) {

    if  (!emitter->particles[i].active) {

      return (int)i;

    }

  }

  return -1;            /* all particles are active */

}

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_particles_init()
	       ---
	       initializes the particle system module. must be called
	       before using any other particle system functions.
	       
	       this sets up any global state needed by the particle
	       system and marks the module as initialized.

*/

int commc_particles_init(void) {

  if  (particles_initialized) {

    commc_report_error(COMMC_FAILURE, __FILE__, __LINE__);

    return 0;                       /* already initialized */

  }

  particles_initialized = 1;

  return 1;                         /* success */

}

/*

         commc_particles_shutdown()
	       ---
	       shuts down the particle system module and cleans up
	       any global resources. should be called when the
	       particle system is no longer needed.

*/

void commc_particles_shutdown(void) {

  if  (!particles_initialized) {

    commc_report_error(COMMC_FAILURE, __FILE__, __LINE__);
    return;

  }

  particles_initialized = 0;

}

/*

         commc_particle_emitter_create()
	       ---
	       creates a new particle emitter with the specified
	       properties. the emitter manages a fixed-size pool
	       of particles and controls their emission rate and
	       characteristics.

	       all particles start as inactive and are activated
	       as needed by the emission logic.

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

) {

  commc_particle_emitter_t* emitter;
  size_t                    i;

  if  (!particles_initialized) {

    commc_report_error(COMMC_FAILURE, __FILE__, __LINE__);
    return NULL;

  }

  if  (capacity == 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  emitter = (commc_particle_emitter_t*)malloc(sizeof(commc_particle_emitter_t));

  if  (!emitter) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* allocate particle array */
  emitter->particles = (commc_particle_t*)malloc(capacity * sizeof(commc_particle_t));

  if  (!emitter->particles) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(emitter);
    return NULL;

  }

  /* initialize emitter properties */

  emitter->capacity             = capacity;
  emitter->count                = 0;
  emitter->position             = position;
  emitter->emission_rate        = emission_rate;
  emitter->time_since_emission  = 0.0f;
  emitter->velocity_min         = velocity_min;
  emitter->velocity_max         = velocity_max;
  emitter->lifetime_min         = lifetime_min;
  emitter->lifetime_max         = lifetime_max;
  emitter->size_min             = size_min;
  emitter->size_max             = size_max;
  emitter->color_start          = color_start;
  emitter->color_end            = color_end;

  /* initialize all particles as inactive */

  for  (i = 0; i < capacity; i++) {

    memset(&emitter->particles[i], 0, sizeof(commc_particle_t));
    emitter->particles[i].active = 0;

  }

  return emitter;

}

/*

         commc_particle_emitter_destroy()
	       ---
	       destroys a particle emitter and frees all associated
	       memory. the emitter pointer becomes invalid after
	       this call and should not be used.

*/

void commc_particle_emitter_destroy(commc_particle_emitter_t* emitter) {

  if  (!emitter) {

    return;

  }

  if  (emitter->particles) {

    free(emitter->particles);

  }

  free(emitter);

}

/*

         commc_particle_emitter_update()
	       ---
	       updates all particles in the emitter based on the
	       elapsed time. this handles particle physics, lifetime
	       management, and new particle emission.

	       dt is the time delta in seconds since the last update.
	       smaller time steps produce smoother animation.

*/

void commc_particle_emitter_update(commc_particle_emitter_t* emitter, float dt) {

  size_t i;

  float  life_ratio;

  if  (!emitter) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  /* update existing particles */

  for  (i = 0; i < emitter->capacity; i++) {

    if  (!emitter->particles[i].active) {

      continue;  /* skip inactive particles */

    }

    /* update particle physics */

    emitter->particles[i].velocity.x += emitter->particles[i].acceleration.x * dt;
    emitter->particles[i].velocity.y += emitter->particles[i].acceleration.y * dt;
    emitter->particles[i].velocity.z += emitter->particles[i].acceleration.z * dt;

    emitter->particles[i].position.x += emitter->particles[i].velocity.x * dt;
    emitter->particles[i].position.y += emitter->particles[i].velocity.y * dt;
    emitter->particles[i].position.z += emitter->particles[i].velocity.z * dt;

    /* update lifetime */

    emitter->particles[i].lifetime -= dt;

    if  (emitter->particles[i].lifetime <= 0.0f) {

      /* particle has expired */

      emitter->particles[i].active = 0;
      emitter->count--;

    } else {

      /* interpolate color based on lifetime */
      life_ratio = 1.0f - (emitter->particles[i].lifetime / emitter->particles[i].initial_lifetime);
      emitter->particles[i].color = interpolate_color(emitter->color_start, emitter->color_end, life_ratio);

    }

  }

  /* handle particle emission */

  emitter->time_since_emission += dt;

  if  (emitter->emission_rate > 0.0f) {

    float emission_interval = 1.0f / emitter->emission_rate;

    while  (emitter->time_since_emission >= emission_interval) {

      commc_particle_emitter_add_particle(emitter);
      emitter->time_since_emission -= emission_interval;

    }

  }

}

/*

         commc_particle_emitter_render()
	       ---
	       renders all active particles in the emitter using
	       the provided graphics context. this is a conceptual
	       function that would need platform-specific rendering
	       code to actually draw the particles.

	       camera_pos can be used for 3D rendering calculations
	       like depth sorting or billboarding.

*/

void commc_particle_emitter_render(commc_particle_emitter_t* emitter, commc_graphics_context_t* context, commc_vec3_t camera_pos) {

  size_t i;

  (void)context;                    /* suppress unused parameter warnings */
  (void)camera_pos;                 /* these would be used in actual implementation */

  if  (!emitter) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  /* conceptual rendering - in a real implementation, this would
     iterate through active particles and draw them using the
     graphics context's rendering functions */

  for  (i = 0; i < emitter->capacity; i++) {

    if  (!emitter->particles[i].active) {

      continue;

    }

    /* example of what rendering might look like:
       
       commc_graphics_draw_sprite(context, 
                                  emitter->particles[i].position,
                                  emitter->particles[i].size,
                                  emitter->particles[i].color);
    */

  }

}

/*

         commc_particle_emitter_add_particle()
	     ---
	     adds a single particle to the emitter if there's
	     space available. the particle is initialized with
	     random properties based on the emitter's settings.

	     this is typically called internally by the update
	     function based on emission rate, but can also be
	     used manually for burst effects.

*/

void commc_particle_emitter_add_particle(commc_particle_emitter_t* emitter) {

  int               particle_index;
  commc_particle_t* particle;

  if  (!emitter) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

    return;

  }

  /* find an inactive particle to reuse */
  particle_index = find_inactive_particle(emitter);

  if  (particle_index == -1) {

    return;             /* no free particles available */

  }

  particle = &emitter->particles[particle_index];

  /* initialize particle properties with random values */
  
  particle->position         = emitter->position;
  particle->velocity         = random_vec3_range(emitter->velocity_min, emitter->velocity_max);
  particle->acceleration.x   = 0.0f;  /* could be randomized for gravity effects */
  particle->acceleration.y   = 0.0f;
  particle->acceleration.z   = 0.0f;
  particle->color            = emitter->color_start;
  particle->size             = random_float_range(emitter->size_min, emitter->size_max);
  particle->lifetime         = random_float_range(emitter->lifetime_min, emitter->lifetime_max);
  particle->initial_lifetime = particle->lifetime;
  particle->active           = 1;

  emitter->count++;

}

/*
	==================================
             --- EOF ---
	==================================
*/