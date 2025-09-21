/*
   ===================================
   C O M M O N - C
   MATH IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- MATH MODULE ---

    implementation of the mathematical utilities.
    see include/commc/math.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>                /* for rand, srand */
#include <time.h>                  /* for time */
#include <string.h>                /* for memset */

#include "commc/math.h"            /* take a guess */
#include "commc/error.h"           /* for error handling */

/*
	==================================
             --- GLOBALS ---
	==================================
*/

static unsigned long commc_rand_state = 1;     /* for custom PRNG */

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

commc_vec2_t commc_vec2_add(commc_vec2_t a, commc_vec2_t b) {

  commc_vec2_t result;
  
  result.x = a.x + b.x;
  result.y = a.y + b.y;

  return result;

}

/*

         commc_vec2_sub()
	       ---
	       subtracts two 2D vectors.

*/

commc_vec2_t commc_vec2_sub(commc_vec2_t a, commc_vec2_t b) {

  commc_vec2_t result;
  
  result.x = a.x - b.x;
  result.y = a.y - b.y;

  return result;

}

/*

         commc_vec2_scale()
	       ---
	       scales a 2D vector by a scalar.

*/

commc_vec2_t commc_vec2_scale(commc_vec2_t v, float s) {

  commc_vec2_t result;
  
  result.x = v.x * s;
  result.y = v.y * s;

  return result;

}

/*

         commc_vec2_dot()
	       ---
	       calculates the dot product of two 2D vectors.

*/

float commc_vec2_dot(commc_vec2_t a, commc_vec2_t b) {

  return a.x * b.x + a.y * b.y;

}

/*

         commc_vec2_length()
	       ---
	       calculates the magnitude (length) of a 2D vector.

*/

float commc_vec2_length(commc_vec2_t v) {

  return (float)sqrt(commc_vec2_dot(v, v));

}

/*

         commc_vec2_normalize()
	       ---
	       normalizes a 2D vector to unit length.

*/

commc_vec2_t commc_vec2_normalize(commc_vec2_t v) {

  float length = commc_vec2_length(v);

  if  (length > COMMC_EPSILON) {

    return commc_vec2_scale(v, 1.0f / length);

  }

  /* return original if length is zero or very small */

  commc_log_debug("OUTPUT: WARNING - Normalizing zero-length vector in commc_vec2_normalize");

  return v;

}

/*

         commc_vec2_dist()
	       ---
	       calculates the distance between two 2D vectors.

*/

float commc_vec2_dist(commc_vec2_t a, commc_vec2_t b) {

  commc_vec2_t diff = commc_vec2_sub(a, b);
  return commc_vec2_length(diff);

}

/*

         commc_vec3_add()
	       ---
	       adds two 3D vectors.

*/

commc_vec3_t commc_vec3_add(commc_vec3_t a, commc_vec3_t b) {

  commc_vec3_t result;
  
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;

  return result;

}

/*

         commc_vec3_sub()
	       ---
	       subtracts two 3D vectors.

*/

commc_vec3_t commc_vec3_sub(commc_vec3_t a, commc_vec3_t b) {

  commc_vec3_t result;
  
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;

  return result;

}

/*

         commc_vec3_scale()
	       ---
	       scales a 3D vector by a scalar.

*/

commc_vec3_t commc_vec3_scale(commc_vec3_t v, float s) {

  commc_vec3_t result;
  
  result.x = v.x * s;
  result.y = v.y * s;
  result.z = v.z * s;

  return result;

}

/*

         commc_vec3_dot()
	       ---
	       calculates the dot product of two 3D vectors.

*/

float commc_vec3_dot(commc_vec3_t a, commc_vec3_t b) {

  return a.x * b.x + a.y * b.y + a.z * b.z;

}

/*

         commc_vec3_cross()
	       ---
	       calculates the cross product of two 3D vectors.

*/

commc_vec3_t commc_vec3_cross(commc_vec3_t a, commc_vec3_t b) {

  commc_vec3_t result;
  
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;

  return result;

}

/*

         commc_vec3_length()
	       ---
	       calculates the magnitude (length) of a 3D vector.

*/

float commc_vec3_length(commc_vec3_t v) {

  return (float)sqrt(commc_vec3_dot(v, v));

}

/*

         commc_vec3_normalize()
	       ---
	       normalizes a 3D vector to unit length.

*/

commc_vec3_t commc_vec3_normalize(commc_vec3_t v) {

  float length = commc_vec3_length(v);

  if  (length > COMMC_EPSILON) {

    return commc_vec3_scale(v, 1.0f / length);

  }

  commc_log_debug("OUTPUT: WARNING - Normalizing zero-length vector in commc_vec3_normalize");

  return v; /* return original if length is zero or very small */

}

/*

         commc_vec3_dist()
	       ---
	       calculates the distance between two 3D vectors.

*/

float commc_vec3_dist(commc_vec3_t a, commc_vec3_t b) {

  commc_vec3_t diff = commc_vec3_sub(a, b);
  return commc_vec3_length(diff);

}

/*

         commc_vec3_reflect()
	       ---
	       reflects a vector off a surface normal.

*/

commc_vec3_t commc_vec3_reflect(commc_vec3_t v, commc_vec3_t normal) {

  float dot_product = commc_vec3_dot(v, normal);
  commc_vec3_t scaled_normal = commc_vec3_scale(normal, 2.0f * dot_product);
  return commc_vec3_sub(v, scaled_normal);

}

/*

         commc_vec3_project()
	       ---
	       projects vector 'a' onto vector 'b'.

*/

commc_vec3_t commc_vec3_project(commc_vec3_t a, commc_vec3_t b) {

  float dot_product = commc_vec3_dot(a, b);
  float b_length_sq = commc_vec3_dot(b, b);

  if  (b_length_sq < COMMC_EPSILON) {

    return commc_vec3_scale(b, 0.0f); /* return zero vector */

  }

  return commc_vec3_scale(b, dot_product / b_length_sq);

}

/*

         commc_mat4_identity()
	       ---
	       returns an identity 4x4 matrix.

*/

commc_mat4_t commc_mat4_identity(void) {

  commc_mat4_t result;
  
  memset(result.m, 0, sizeof(result.m));
  result.m[0]  = 1.0f;
  result.m[5]  = 1.0f;
  result.m[10] = 1.0f;
  result.m[15] = 1.0f;

  return result;

}

/*

         commc_mat4_multiply()
	       ---
	       multiplies two 4x4 matrices.

*/

commc_mat4_t commc_mat4_multiply(commc_mat4_t a, commc_mat4_t b) {

  commc_mat4_t result;
  int          i, j;

  for  (i = 0; i < 4; i++) {

    for  (j = 0; j < 4; j++) {

      result.m[i + j * 4] = a.m[i + 0 * 4] * b.m[0 + j * 4] +
                            a.m[i + 1 * 4] * b.m[1 + j * 4] +
                            a.m[i + 2 * 4] * b.m[2 + j * 4] +
                            a.m[i + 3 * 4] * b.m[3 + j * 4];

    }

  }

  return result;

}

/*

         commc_mat4_transpose()
	       ---
	       transposes a 4x4 matrix.

*/

commc_mat4_t commc_mat4_transpose(commc_mat4_t m) {

  commc_mat4_t result;
  int          i, j;

  for  (i = 0; i < 4; i++) {

    for  (j = 0; j < 4; j++) {

      result.m[j + i * 4] = m.m[i + j * 4];

    }

  }

  return result;

}

/*

         commc_mat4_inverse()
	       ---
	       calculates the inverse of a 4x4 matrix.
	       returns identity if inverse cannot be found.
	       (using Cramer's rule for general inverse)

*/

commc_mat4_t commc_mat4_inverse(commc_mat4_t m) {

  commc_mat4_t inv;
  float        det;
  int          i;

  inv.m[0] = m.m[5]  * m.m[10] * m.m[15] - 
             m.m[5]  * m.m[11] * m.m[14] - 
             m.m[9]  * m.m[6]  * m.m[15] + 
             m.m[9]  * m.m[7]  * m.m[14] +
             m.m[13] * m.m[6]  * m.m[11] - 
             m.m[13] * m.m[7]  * m.m[10];

  inv.m[4] = -m.m[4]  * m.m[10] * m.m[15] + 
              m.m[4]  * m.m[11] * m.m[14] + 
              m.m[8]  * m.m[6]  * m.m[15] - 
              m.m[8]  * m.m[7]  * m.m[14] - 
              m.m[12] * m.m[6]  * m.m[11] + 
              m.m[12] * m.m[7]  * m.m[10];

  inv.m[8] = m.m[4]  * m.m[9] * m.m[15] - 
             m.m[4]  * m.m[11] * m.m[13] - 
             m.m[8]  * m.m[5] * m.m[15] + 
             m.m[8]  * m.m[7] * m.m[13] + 
             m.m[12] * m.m[5] * m.m[11] - 
             m.m[12] * m.m[7] * m.m[9];

  inv.m[12] = -m.m[4]  * m.m[9] * m.m[14] + 
               m.m[4]  * m.m[10] * m.m[13] +
               m.m[8]  * m.m[5] * m.m[14] - 
               m.m[8]  * m.m[6] * m.m[13] - 
               m.m[12] * m.m[5] * m.m[10] + 
               m.m[12] * m.m[6] * m.m[9];

  inv.m[1] = -m.m[1]  * m.m[10] * m.m[15] + 
              m.m[1]  * m.m[11] * m.m[14] + 
              m.m[9]  * m.m[2] * m.m[15] - 
              m.m[9]  * m.m[3] * m.m[14] - 
              m.m[13] * m.m[2] * m.m[11] + 
              m.m[13] * m.m[3] * m.m[10];

  inv.m[5] = m.m[0]  * m.m[10] * m.m[15] - 
             m.m[0]  * m.m[11] * m.m[14] - 
             m.m[8]  * m.m[2] * m.m[15] + 
             m.m[8]  * m.m[3] * m.m[14] + 
             m.m[12] * m.m[2] * m.m[11] - 
             m.m[12] * m.m[3] * m.m[10];

  inv.m[9] = -m.m[0]  * m.m[9] * m.m[15] + 
              m.m[0]  * m.m[11] * m.m[13] + 
              m.m[8]  * m.m[1] * m.m[15] - 
              m.m[8]  * m.m[3] * m.m[13] - 
              m.m[12] * m.m[1] * m.m[11] + 
              m.m[12] * m.m[3] * m.m[9];

  inv.m[13] = m.m[0]  * m.m[9] * m.m[14] - 
              m.m[0]  * m.m[10] * m.m[13] - 
              m.m[8]  * m.m[1] * m.m[14] + 
              m.m[8]  * m.m[2] * m.m[13] + 
              m.m[12] * m.m[1] * m.m[10] - 
              m.m[12] * m.m[2] * m.m[9];

  inv.m[2] = m.m[1]  * m.m[6] * m.m[15] - 
             m.m[1]  * m.m[7] * m.m[14] - 
             m.m[5]  * m.m[2] * m.m[15] + 
             m.m[5]  * m.m[3] * m.m[14] + 
             m.m[13] * m.m[2] * m.m[7] - 
             m.m[13] * m.m[3] * m.m[6];

  inv.m[6] = -m.m[0]  * m.m[6] * m.m[15] + 
              m.m[0]  * m.m[7] * m.m[14] + 
              m.m[4]  * m.m[2] * m.m[15] - 
              m.m[4]  * m.m[3] * m.m[14] - 
              m.m[12] * m.m[2] * m.m[7] + 
              m.m[12] * m.m[3] * m.m[6];

  inv.m[10] = m.m[0]  * m.m[5] * m.m[15] - 
              m.m[0]  * m.m[7] * m.m[13] - 
              m.m[4]  * m.m[1] * m.m[15] + 
              m.m[4]  * m.m[3] * m.m[13] + 
              m.m[12] * m.m[1] * m.m[7] - 
              m.m[12] * m.m[3] * m.m[5];

  inv.m[14] = -m.m[0]  * m.m[5] * m.m[14] + 
               m.m[0]  * m.m[6] * m.m[13] + 
               m.m[4]  * m.m[1] * m.m[14] - 
               m.m[4]  * m.m[2] * m.m[13] - 
               m.m[12] * m.m[1] * m.m[6] + 
               m.m[12] * m.m[2] * m.m[5];

  inv.m[3] = -m.m[1] * m.m[6] * m.m[11] + 
              m.m[1] * m.m[7] * m.m[10] + 
              m.m[5] * m.m[2] * m.m[11] - 
              m.m[5] * m.m[3] * m.m[10] - 
              m.m[9] * m.m[2] * m.m[7] + 
              m.m[9] * m.m[3] * m.m[6];

  inv.m[7] = m.m[0] * m.m[6] * m.m[11] - 
             m.m[0] * m.m[7] * m.m[10] - 
             m.m[4] * m.m[2] * m.m[11] + 
             m.m[4] * m.m[3] * m.m[10] + 
             m.m[8] * m.m[2] * m.m[7] - 
             m.m[8] * m.m[3] * m.m[6];

  inv.m[11] = -m.m[0] * m.m[5] * m.m[11] + 
               m.m[0] * m.m[7] * m.m[9] + 
               m.m[4] * m.m[1] * m.m[11] - 
               m.m[4] * m.m[3] * m.m[9] - 
               m.m[8] * m.m[1] * m.m[7] + 
               m.m[8] * m.m[3] * m.m[5];

  inv.m[15] = m.m[0] * m.m[5] * m.m[10] - 
              m.m[0] * m.m[6] * m.m[9] - 
              m.m[4] * m.m[1] * m.m[10] + 
              m.m[4] * m.m[2] * m.m[9] + 
              m.m[8] * m.m[1] * m.m[6] - 
              m.m[8] * m.m[2] * m.m[5];

  det = m.m[0] * inv.m[0] + m.m[1] * inv.m[4] + m.m[2] * inv.m[8] + m.m[3] * inv.m[12];

  if  (fabs(det) < COMMC_EPSILON) {

    /* return identity if singular */

    return commc_mat4_identity();

  }

  det = 1.0f / det;

  for  (i = 0; i < 16; i++) {

    inv.m[i] = inv.m[i] * det;

  }

  return inv;

}

/*

         commc_mat4_translate()
	       ---
	       creates a translation matrix.

*/

commc_mat4_t commc_mat4_translate(commc_vec3_t v) {

  commc_mat4_t result = commc_mat4_identity();
  
  result.m[12] = v.x;
  result.m[13] = v.y;
  result.m[14] = v.z;

  return result;

}

/*

         commc_mat4_rotate_x()
	       ---
	       creates a rotation matrix around the X-axis.

*/

commc_mat4_t commc_mat4_rotate_x(float angle_radians) {

  commc_mat4_t result = commc_mat4_identity();

  float  c      = (float)cos(angle_radians);
  float  s      = (float)sin(angle_radians);

  result.m[5]   = c;
  result.m[6]   = s;
  result.m[9]   = -s;
  result.m[10]  = c;

  return result;

}

/*

         commc_mat4_rotate_y()
	       ---
	       creates a rotation matrix around the Y-axis.

*/

commc_mat4_t commc_mat4_rotate_y(float angle_radians) {

  commc_mat4_t result = commc_mat4_identity();

  float  c      = (float)cos(angle_radians);
  float  s      = (float)sin(angle_radians);

  result.m[0]   = c;
  result.m[2]   = -s;
  result.m[8]   = s;
  result.m[10]  = c;

  return result;

}

/*

         commc_mat4_rotate_z()
	       ---
	       creates a rotation matrix around the Z-axis.

*/

commc_mat4_t commc_mat4_rotate_z(float angle_radians) {

  commc_mat4_t result = commc_mat4_identity();

  float  c      = (float)cos(angle_radians);
  float  s      = (float)sin(angle_radians);

  result.m[0]  = c;
  result.m[1]  = s;
  result.m[4]  = -s;
  result.m[5]  = c;

  return result;

}

/*

         commc_mat4_scale()
	       ---
	       creates a scaling matrix.

*/

commc_mat4_t commc_mat4_scale(commc_vec3_t v) {

  commc_mat4_t result = commc_mat4_identity();
  
  result.m[0]  = v.x;
  result.m[5]  = v.y;
  result.m[10] = v.z;

  return result;

}

/*

         commc_mat4_perspective()
	       ---
	       creates a perspective projection matrix.

*/

commc_mat4_t commc_mat4_perspective(float fov_radians, float aspect, float near_plane, float far_plane) {

  commc_mat4_t result;
  float        f;

  memset(result.m, 0, sizeof(result.m));

  f = 1.0f / (float)tan(fov_radians / 2.0f);

  result.m[0]  = f / aspect;
  result.m[5]  = f;
  result.m[10] = (far_plane + near_plane) / (near_plane - far_plane);
  result.m[11] = -1.0f;
  result.m[14] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);

  return result;

}

/*

         commc_mat4_ortho()
	       ---
	       creates an orthographic projection matrix.

*/

commc_mat4_t commc_mat4_ortho(float left, float right, float bottom, float top, float near_plane, float far_plane) {

  commc_mat4_t result;
  
  memset(result.m, 0, sizeof(result.m));

  result.m[0]  = 2.0f / (right - left);
  result.m[5]  = 2.0f / (top - bottom);
  result.m[10] = -2.0f / (far_plane - near_plane);
  result.m[12] = -(right + left) / (right - left);
  result.m[13] = -(top + bottom) / (top - bottom);
  result.m[14] = -(far_plane + near_plane) / (far_plane - near_plane);
  result.m[15] = 1.0f;

  return result;

}

/*

         commc_mat4_look_at()
	       ---
	       creates a view matrix (camera transformation).

*/

commc_mat4_t commc_mat4_look_at(commc_vec3_t eye, commc_vec3_t center, commc_vec3_t up) {

  commc_mat4_t result;
  commc_vec3_t f, s, u;

  f = commc_vec3_normalize(commc_vec3_sub(center, eye));
  s = commc_vec3_normalize(commc_vec3_cross(f, up));
  u = commc_vec3_cross(s, f);

  result.m[0]  = s.x;
  result.m[4]  = s.y;
  result.m[8]  = s.z;
  result.m[12] = -commc_vec3_dot(s, eye);

  result.m[1]  = u.x;
  result.m[5]  = u.y;
  result.m[9]  = u.z;
  result.m[13] = -commc_vec3_dot(u, eye);

  result.m[2]  = -f.x;
  result.m[6]  = -f.y;
  result.m[10] = -f.z;
  result.m[14] = commc_vec3_dot(f, eye);

  result.m[3]  = 0.0f;
  result.m[7]  = 0.0f;
  result.m[11] = 0.0f;
  result.m[15] = 1.0f;

  return result;

}

/*

         commc_quat_identity()
	       ---
	       returns an identity quaternion.

*/

commc_quat_t commc_quat_identity(void) {

  commc_quat_t result;
  
  result.x = 0.0f;
  result.y = 0.0f;
  result.z = 0.0f;
  result.w = 1.0f;

  return result;

}

/*

         commc_quat_from_axis_angle()
	       ---
	       creates a quaternion from an axis and an angle.

*/

commc_quat_t commc_quat_from_axis_angle(commc_vec3_t axis, float angle_radians) {

  commc_quat_t result;
  float        half_angle = angle_radians * 0.5f;
  float        s          = (float)sin(half_angle);

  result.x = axis.x * s;
  result.y = axis.y * s;
  result.z = axis.z * s;
  result.w = (float)cos(half_angle);

  return result;

}

/*

         commc_quat_multiply()
	       ---
	       multiplies two quaternions.

*/

commc_quat_t commc_quat_multiply(commc_quat_t q1, commc_quat_t q2) {

  commc_quat_t result;
  
  result.x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
  result.y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
  result.z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
  result.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;

  return result;

}

/*

         commc_quat_to_mat4()
	       ---
	       converts a quaternion to a 4x4 rotation matrix.

*/

commc_mat4_t commc_quat_to_mat4(commc_quat_t q) {

  commc_mat4_t result = commc_mat4_identity();

  float  xx     = q.x * q.x;
  float  yy     = q.y * q.y;
  float  zz     = q.z * q.z;
  float  xy     = q.x * q.y;
  float  zw     = q.z * q.w;
  float  xz     = q.x * q.z;
  float  yw     = q.y * q.w;
  float  yz     = q.y * q.z;
  float  xw     = q.x * q.w;

  result.m[0]  = 1.0f - 2.0f * (yy + zz);
  result.m[1]  = 2.0f * (xy + zw);
  result.m[2]  = 2.0f * (xz - yw);

  result.m[4]  = 2.0f * (xy - zw);
  result.m[5]  = 1.0f - 2.0f * (xx + zz);
  result.m[6]  = 2.0f * (yz + xw);

  result.m[8]  = 2.0f * (xz + yw);
  result.m[9]  = 2.0f * (yz - xw);
  result.m[10] = 1.0f - 2.0f * (xx + yy);

  return result;

}

/*

         commc_quat_normalize()
	       ---
	       normalizes a quaternion.

*/

commc_quat_t commc_quat_normalize(commc_quat_t q) {

  float length_sq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
  float inv_length;

  if  (length_sq < COMMC_EPSILON) {

    commc_log_debug("OUTPUT: WARNING - Normalizing zero-length quaternion in commc_quat_normalize");
    return commc_quat_identity();

  }

  inv_length = commc_fast_inverse_sqrt(length_sq);

  q.x *= inv_length;
  q.y *= inv_length;
  q.z *= inv_length;
  q.w *= inv_length;

  return q;

}

/*

         commc_fast_inverse_sqrt()
	       ---
	       computes the fast inverse square root (Q_rsqrt).
	       this is the famous "quake III arena" algorithm.

*/

float commc_fast_inverse_sqrt(float number) {

  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y  = number;
  i  = *(long*)&y;
  i  = 0x5f3759df - (i >> 1);
  y  = *(float*)&i;
  y  = y * (threehalfs - (x2 * y * y));
  /* y  = y * (threehalfs - (x2 * y * y)); */ /* 2nd iteration, can be removed */

  return y;

}

/*

         commc_min()
	       ---
	       returns the minimum of two floats.

*/

float commc_min(float a, float b) {

  return a < b ? a : b;

}

/*

         commc_max()
	       ---
	       returns the maximum of two floats.

*/

float commc_max(float a, float b) {

  return a > b ? a : b;

}

/*

         commc_clamp()
	       ---
	       clamps a value between a minimum and maximum.

*/

float commc_clamp(float value, float min_val, float max_val) {

  return commc_max(min_val, commc_min(value, max_val));

}

/*

         commc_lerp()
	       ---
	       linearly interpolates between two values.

*/

float commc_lerp(float a, float b, float t) {

  return a + t * (b - a);

}

/*

         commc_rand_seed()
	       ---
	       seeds the custom random number generator.

*/

void commc_rand_seed(unsigned long seed) {

  commc_rand_state = seed == 0 ? (unsigned long)time(NULL) : seed;

}

/*

         commc_rand_float()
	       ---
	       generates a random float between 0.0 and 1.0
	       using a simple Xorshift-like PRNG.

*/

float commc_rand_float(void) {

  /* xorshift algorithm */

  commc_rand_state ^= commc_rand_state << 13;
  commc_rand_state ^= commc_rand_state >> 17;
  commc_rand_state ^= commc_rand_state << 5;

  return (float)commc_rand_state / (float)0xFFFFFFFFUL; /* scale to 0-1 */

}

/*

         commc_rand_int()
	       ---
	       generates a random integer within a specified range.

*/

int commc_rand_int(int min_val, int max_val) {

  if  (min_val > max_val) {

    int temp = min_val;
    min_val  = max_val;
    max_val  = temp;

  }

  return min_val + (int)(commc_rand_float() * (max_val - min_val + 1));

}

/*
	==================================
             --- EOF ---
	==================================
*/
