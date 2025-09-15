/*
   ===================================
   C O M M O N - C
   MATH UTILITIES MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- MATH MODULE ---

    this module provides common mathematical utilities
    for 2D/3D vectors and 4x4 matrices, essential for
    game development and graphics programming.

    all functions are designed for C89 compliance.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_MATH_H
#define COMMC_MATH_H

#include <math.h> 								  /* for sin, cos, sqrt, tan */

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define COMMC_EPSILON 0.00001f     /* small value for float comparisons */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* 2D vector. */

typedef struct {

  float x;
  float y;

} commc_vec2_t;

/* 3D vector. */

typedef struct {

  float x;
  float y;
  float z;

} commc_vec3_t;

/* 4x4 matrix (column-major for opengl compatibility). */

typedef struct {

  float m[16]; 								/* 4x4 matrix elements */

} commc_mat4_t;

/* quaternion for rotations. */

typedef struct {

  float x;
  float y;
  float z;
  float w;

} commc_quat_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_vec2_add()
	       ---
	       adds two 2D vectors.

*/

commc_vec2_t commc_vec2_add(commc_vec2_t a, commc_vec2_t b);

/*

         commc_vec2_sub()
	       ---
	       subtracts two 2D vectors.

*/

commc_vec2_t commc_vec2_sub(commc_vec2_t a, commc_vec2_t b);

/*

         commc_vec2_scale()
	       ---
	       scales a 2D vector by a scalar.

*/

commc_vec2_t commc_vec2_scale(commc_vec2_t v, float s);

/*

         commc_vec2_dot()
	       ---
	       calculates the dot product of two 2D vectors.

*/

float commc_vec2_dot(commc_vec2_t a, commc_vec2_t b);

/*

         commc_vec2_length()
	       ---
	       calculates the magnitude (length) of a 2D vector.

*/

float commc_vec2_length(commc_vec2_t v);

/*

         commc_vec2_normalize()
	       ---
	       normalizes a 2D vector to unit length.

*/

commc_vec2_t commc_vec2_normalize(commc_vec2_t v);

/*

         commc_vec2_dist()
	       ---
	       calculates the distance between two 2D vectors.

*/

float commc_vec2_dist(commc_vec2_t a, commc_vec2_t b);

/*

         commc_vec3_add()
	       ---
	       adds two 3D vectors.

*/

commc_vec3_t commc_vec3_add(commc_vec3_t a, commc_vec3_t b);

/*

         commc_vec3_sub()
	       ---
	       subtracts two 3D vectors.

*/

commc_vec3_t commc_vec3_sub(commc_vec3_t a, commc_vec3_t b);

/*

         commc_vec3_scale()
	       ---
	       scales a 3D vector by a scalar.

*/

commc_vec3_t commc_vec3_scale(commc_vec3_t v, float s);

/*

         commc_vec3_dot()
	       ---
	       calculates the dot product of two 3D vectors.

*/

float commc_vec3_dot(commc_vec3_t a, commc_vec3_t b);

/*

         commc_vec3_cross()
	       ---
	       calculates the cross product of two 3D vectors.

*/

commc_vec3_t commc_vec3_cross(commc_vec3_t a, commc_vec3_t b);

/*

         commc_vec3_length()
	       ---
	       calculates the magnitude (length) of a 3D vector.

*/

float commc_vec3_length(commc_vec3_t v);

/*

         commc_vec3_normalize()
	       ---
	       normalizes a 3D vector to unit length.

*/

commc_vec3_t commc_vec3_normalize(commc_vec3_t v);

/*

         commc_vec3_dist()
	       ---
	       calculates the distance between two 3D vectors.

*/

float commc_vec3_dist(commc_vec3_t a, commc_vec3_t b);

/*

         commc_vec3_reflect()
	       ---
	       reflects a vector off a surface normal.

*/

commc_vec3_t commc_vec3_reflect(commc_vec3_t v, commc_vec3_t normal);

/*

         commc_vec3_project()
	       ---
	       projects vector 'a' onto vector 'b'.

*/

commc_vec3_t commc_vec3_project(commc_vec3_t a, commc_vec3_t b);

/*

         commc_mat4_identity()
	       ---
	       returns an identity 4x4 matrix.

*/

commc_mat4_t commc_mat4_identity(void);

/*

         commc_mat4_multiply()
	       ---
	       multiplies two 4x4 matrices.

*/

commc_mat4_t commc_mat4_multiply(commc_mat4_t a, commc_mat4_t b);

/*

         commc_mat4_transpose()
	       ---
	       transposes a 4x4 matrix.

*/

commc_mat4_t commc_mat4_transpose(commc_mat4_t m);

/*

         commc_mat4_inverse()
	       ---
	       calculates the inverse of a 4x4 matrix.
	       returns identity if inverse cannot be found.

*/

commc_mat4_t commc_mat4_inverse(commc_mat4_t m);

/*

         commc_mat4_translate()
	       ---
	       creates a translation matrix.

*/

commc_mat4_t commc_mat4_translate(commc_vec3_t v);

/*

         commc_mat4_rotate_x()
	       ---
	       creates a rotation matrix around the X-axis.

*/

commc_mat4_t commc_mat4_rotate_x(float angle_radians);

/*

         commc_mat4_rotate_y()
	       ---
	       creates a rotation matrix around the Y-axis.

*/

commc_mat4_t commc_mat4_rotate_y(float angle_radians);

/*

         commc_mat4_rotate_z()
	       ---
	       creates a rotation matrix around the Z-axis.

*/

commc_mat4_t commc_mat4_rotate_z(float angle_radians);

/*

         commc_mat4_scale()
	       ---
	       creates a scaling matrix.

*/

commc_mat4_t commc_mat4_scale(commc_vec3_t v);

/*

         commc_mat4_perspective()
	       ---
	       creates a perspective projection matrix.

*/

commc_mat4_t commc_mat4_perspective(float fov_radians, float aspect, float near_plane, float far_plane);

/*

         commc_mat4_ortho()
	       ---
	       creates an orthographic projection matrix.

*/

commc_mat4_t commc_mat4_ortho(float left, float right, float bottom, float top, float near_plane, float far_plane);

/*

         commc_mat4_look_at()
	       ---
	       creates a view matrix (camera transformation).

*/

commc_mat4_t commc_mat4_look_at(commc_vec3_t eye, commc_vec3_t center, commc_vec3_t up);

/*

         commc_quat_identity()
	       ---
	       returns an identity quaternion.

*/

commc_quat_t commc_quat_identity(void);

/*

         commc_quat_from_axis_angle()
	       ---
	       creates a quaternion from an axis and an angle.

*/

commc_quat_t commc_quat_from_axis_angle(commc_vec3_t axis, float angle_radians);

/*

         commc_quat_multiply()
	       ---
	       multiplies two quaternions.

*/

commc_quat_t commc_quat_multiply(commc_quat_t q1, commc_quat_t q2);

/*

         commc_quat_to_mat4()
	       ---
	       converts a quaternion to a 4x4 rotation matrix.

*/

commc_mat4_t commc_quat_to_mat4(commc_quat_t q);

/*

         commc_quat_normalize()
	       ---
	       normalizes a quaternion.

*/

commc_quat_t commc_quat_normalize(commc_quat_t q);

/*

         commc_fast_inverse_sqrt()
	       ---
	       computes the fast inverse square root (Q_rsqrt).

*/

float commc_fast_inverse_sqrt(float number);

/*

         commc_min()
	       ---
	       returns the minimum of two floats.

*/

float commc_min(float a, float b);

/*

         commc_max()
	       ---
	       returns the maximum of two floats.

*/

float commc_max(float a, float b);

/*

         commc_clamp()
	       ---
	       clamps a value between a minimum and maximum.

*/

float commc_clamp(float value, float min_val, float max_val);

/*

         commc_lerp()
	       ---
	       linearly interpolates between two values.

*/

float commc_lerp(float a, float b, float t);

/*

         commc_rand_seed()
	       ---
	       seeds the custom random number generator.

*/

void commc_rand_seed(unsigned long seed);

/*

         commc_rand_float()
	       ---
	       generates a random float between 0.0 and 1.0.

*/

float commc_rand_float(void);

/*

         commc_rand_int()
	       ---
	       generates a random integer within a specified range.

*/

int commc_rand_int(int min_val, int max_val);

#endif /* COMMC_MATH_H */

/*
	==================================
             --- EOF ---
	==================================
*/
