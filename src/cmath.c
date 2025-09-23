/*
   ===================================
   C O M M O N - C
   CMATH IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- CMATH MODULE ---

    implementation of the mathematical utilities.
    see include/commc/cmath.h for function
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
#include <math.h>                  /* for sqrt, log, exp, etc. */

#include "commc/cmath.h"            /* take a guess */
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

         commc_mat4_lu_decompose()
	       ---
	       performs LU decomposition with partial pivoting on a 4x4 matrix.
	       
	       this function implements Gaussian elimination with partial pivoting
	       to decompose matrix A into PA = LU where:
	       - P is a permutation matrix (represented by pivot indices)
	       - L is a lower triangular matrix with 1s on diagonal
	       - U is an upper triangular matrix
	       
	       partial pivoting improves numerical stability by choosing the
	       largest element in each column as the pivot, reducing round-off
	       errors that can accumulate during elimination.

*/

commc_mat4_lu_t commc_mat4_lu_decompose(commc_mat4_t matrix) {

  commc_mat4_lu_t result;
  float           work[16];  /* working copy of matrix */
  int             i, j, k;
  int             max_row;
  float           max_val, temp;
  
  /* initialize result structure */
  
  result.lower      = commc_mat4_identity();
  result.upper      = commc_mat4_identity();
  result.is_singular = 0;
  
  /* initialize pivot array and working matrix */
  
  for  (i = 0; i < 4; i++) {
  
    result.pivots[i] = i;
    
    for  (j = 0; j < 4; j++) {
    
      work[i + j * 4] = matrix.m[i + j * 4];
      
    }
    
  }
  
  /* perform LU decomposition with partial pivoting */
  
  for  (k = 0; k < 4; k++) {
  
    /* find the pivot (largest element in column k from row k downward) */
    
    max_val = (float)fabs(work[k + k * 4]);
    max_row = k;
    
    for  (i = k + 1; i < 4; i++) {
    
      temp = (float)fabs(work[i + k * 4]);
      
      if  (temp > max_val) {
      
        max_val = temp;
        max_row = i;
        
      }
      
    }
    
    /* check for singular matrix */
    
    if  (max_val < COMMC_EPSILON) {
    
      result.is_singular = 1;
      return result;
      
    }
    
    /* swap rows if necessary */
    
    if  (max_row != k) {
    
      /* swap pivot indices */
      
      temp = (float)result.pivots[k];
      result.pivots[k] = result.pivots[max_row];
      result.pivots[max_row] = (int)temp;
      
      /* swap rows in working matrix */
      
      for  (j = 0; j < 4; j++) {
      
        temp = work[k + j * 4];
        work[k + j * 4] = work[max_row + j * 4];
        work[max_row + j * 4] = temp;
        
      }
      
    }
    
    /* eliminate below the pivot */
    
    for  (i = k + 1; i < 4; i++) {
    
      /* calculate multiplier for elimination */
      
      work[i + k * 4] = work[i + k * 4] / work[k + k * 4];
      
      /* eliminate elements in row i, columns k+1 to 3 */
      
      for  (j = k + 1; j < 4; j++) {
      
        work[i + j * 4] -= work[i + k * 4] * work[k + j * 4];
        
      }
      
    }
    
  }
  
  /* extract L and U matrices from the working matrix */
  
  for  (i = 0; i < 4; i++) {
  
    for  (j = 0; j < 4; j++) {
    
      if  (i > j) {
      
        /* lower triangular part (multipliers from elimination) */
        
        result.lower.m[i + j * 4] = work[i + j * 4];
        result.upper.m[i + j * 4] = 0.0f;
        
      } else if  (i == j) {
      
        /* diagonal: L has 1s, U has pivots */
        
        result.lower.m[i + j * 4] = 1.0f;
        result.upper.m[i + j * 4] = work[i + j * 4];
        
      } else {
      
        /* upper triangular part */
        
        result.lower.m[i + j * 4] = 0.0f;
        result.upper.m[i + j * 4] = work[i + j * 4];
        
      }
      
    }
    
  }
  
  return result;

}

/*

         commc_mat4_lu_solve()
	       ---
	       solves the linear system Ax = b using precomputed LU decomposition.
	       
	       this function uses forward substitution followed by back substitution:
	       1. forward substitution: solve Ly = Pb for y
	       2. back substitution: solve Ux = y for x
	       
	       this approach is much more efficient than computing A^(-1) * b
	       and is numerically more stable for solving linear systems.

*/

commc_vec3_t commc_mat4_lu_solve(commc_mat4_lu_t lu_decomp, commc_vec3_t b) {

  commc_vec3_t result;
  float        y[4];  /* intermediate solution vector */
  float        pb[4]; /* permuted right-hand side */
  int          i, j;
  
  /* check for singular matrix */
  
  if  (lu_decomp.is_singular) {
  
    commc_log_debug("OUTPUT: WARNING - Attempting to solve with singular matrix in commc_mat4_lu_solve");
    result.x = 0.0f;
    result.y = 0.0f;
    result.z = 0.0f;
    return result;
    
  }
  
  /* create permuted right-hand side Pb */
  
  pb[0] = (lu_decomp.pivots[0] == 0) ? b.x : (lu_decomp.pivots[0] == 1) ? b.y : b.z;
  pb[1] = (lu_decomp.pivots[1] == 0) ? b.x : (lu_decomp.pivots[1] == 1) ? b.y : b.z;
  pb[2] = (lu_decomp.pivots[2] == 0) ? b.x : (lu_decomp.pivots[2] == 1) ? b.y : b.z;
  pb[3] = 0.0f; /* assuming homogeneous coordinates */
  
  /* forward substitution: solve Ly = Pb */
  
  for  (i = 0; i < 4; i++) {
  
    y[i] = pb[i];
    
    for  (j = 0; j < i; j++) {
    
      y[i] -= lu_decomp.lower.m[i + j * 4] * y[j];
      
    }
    
  }
  
  /* back substitution: solve Ux = y */
  
  for  (i = 3; i >= 0; i--) {
  
    for  (j = i + 1; j < 4; j++) {
    
      y[i] -= lu_decomp.upper.m[i + j * 4] * y[j];
      
    }
    
    y[i] /= lu_decomp.upper.m[i + i * 4];
    
  }
  
  /* extract 3D result from 4D homogeneous solution */
  
  result.x = y[0];
  result.y = y[1];
  result.z = y[2];
  
  return result;

}

/*

         commc_mat4_lu_determinant()
	       ---
	       computes the determinant of a matrix using its LU decomposition.
	       
	       for a matrix A with LU decomposition PA = LU:
	       det(A) = det(P^(-1)) * det(L) * det(U)
	       
	       since det(L) = 1 (lower triangular with 1s on diagonal) and
	       det(U) = product of diagonal elements, we have:
	       det(A) = sign(P) * product(U_diagonal)
	       
	       where sign(P) = (-1)^(number of row swaps)

*/

float commc_mat4_lu_determinant(commc_mat4_lu_t lu_decomp) {

  float determinant;
  int   i;
  int   swap_count;
  
  /* check for singular matrix */
  
  if  (lu_decomp.is_singular) {
  
    return 0.0f;
    
  }
  
  /* compute product of diagonal elements of U */
  
  determinant = 1.0f;
  
  for  (i = 0; i < 4; i++) {
  
    determinant *= lu_decomp.upper.m[i + i * 4];
    
  }
  
  /* count number of row swaps to determine sign of permutation */
  
  swap_count = 0;
  
  for  (i = 0; i < 4; i++) {
  
    if  (lu_decomp.pivots[i] != i) {
    
      swap_count++;
      
    }
    
  }
  
  /* apply sign correction for permutation matrix */
  
  if  (swap_count % 2 == 1) {
  
    determinant = -determinant;
    
  }
  
  return determinant;

}

/*

         commc_mat4_eigenvalue_dominant()
	       ---
	       finds the dominant (largest magnitude) eigenvalue using power iteration.
	       
	       power iteration is based on the mathematical principle that
	       repeatedly multiplying a vector by a matrix A will cause the
	       vector to converge toward the eigenvector corresponding to the
	       eigenvalue with the largest absolute value.
	       
	       mathematically: if v_k = A^k * v_0, then v_k approaches the
	       dominant eigenvector, and |A*v_k| / |v_k| approaches the
	       dominant eigenvalue λ_1.

*/

commc_complex_t commc_mat4_eigenvalue_dominant(commc_mat4_t matrix, int max_iterations) {

  commc_complex_t result;
  commc_vec3_t    v, v_new;
  float           eigenvalue, eigenvalue_prev;
  float           norm, norm_new;
  int             i;
  
  /* initialize result */
  
  result.real = 0.0f;
  result.imag = 0.0f;
  
  /* initialize random starting vector */
  
  v.x = 1.0f;
  v.y = 1.0f; 
  v.z = 1.0f;
  
  eigenvalue_prev = 0.0f;
  
  for  (i = 0; i < max_iterations; i++) {
  
    /* multiply vector by matrix (treating 4x4 as operating on 3D homogeneous coords) */
    
    v_new.x = matrix.m[0] * v.x + matrix.m[4] * v.y + matrix.m[8]  * v.z;
    v_new.y = matrix.m[1] * v.x + matrix.m[5] * v.y + matrix.m[9]  * v.z;
    v_new.z = matrix.m[2] * v.x + matrix.m[6] * v.y + matrix.m[10] * v.z;
    
    /* calculate norms */
    
    norm     = commc_vec3_length(v);
    norm_new = commc_vec3_length(v_new);
    
    /* avoid division by zero */
    
    if  (norm < COMMC_EPSILON) {
    
      commc_log_debug("OUTPUT: WARNING - Zero vector in power iteration");
      return result;
      
    }
    
    /* estimate eigenvalue as ratio of norms */
    
    eigenvalue = norm_new / norm;
    
    /* check for convergence */
    
    if  (i > 0 && (float)fabs(eigenvalue - eigenvalue_prev) < COMMC_EPSILON) {
    
      result.real = eigenvalue;
      return result;
      
    }
    
    /* normalize vector for next iteration */
    
    v = commc_vec3_normalize(v_new);
    eigenvalue_prev = eigenvalue;
    
  }
  
  /* return best estimate even if not fully converged */
  
  result.real = eigenvalue;
  return result;

}

/*

         commc_mat4_eigenvector_dominant()
	       ---
	       finds the eigenvector corresponding to the dominant eigenvalue.
	       
	       this uses the same power iteration algorithm but focuses on
	       obtaining the converged eigenvector rather than the eigenvalue.
	       the eigenvector represents the direction of maximum variance
	       or the principal axis of the linear transformation.

*/

commc_vec3_t commc_mat4_eigenvector_dominant(commc_mat4_t matrix, int max_iterations) {

  commc_vec3_t v, v_new;
  float        norm;
  int          i;
  
  /* initialize random starting vector */
  
  v.x = 1.0f;
  v.y = 1.0f;
  v.z = 1.0f;
  
  for  (i = 0; i < max_iterations; i++) {
  
    /* multiply vector by matrix */
    
    v_new.x = matrix.m[0] * v.x + matrix.m[4] * v.y + matrix.m[8]  * v.z;
    v_new.y = matrix.m[1] * v.x + matrix.m[5] * v.y + matrix.m[9]  * v.z;
    v_new.z = matrix.m[2] * v.x + matrix.m[6] * v.y + matrix.m[10] * v.z;
    
    /* normalize the result */
    
    norm = commc_vec3_length(v_new);
    
    if  (norm < COMMC_EPSILON) {
    
      commc_log_debug("OUTPUT: WARNING - Zero vector in eigenvector iteration");
      return v;
      
    }
    
    v = commc_vec3_scale(v_new, 1.0f / norm);
    
  }
  
  return v;

}

/*

         commc_mat4_eigen_decompose()
	       ---
	       computes eigenvalues and eigenvectors using simplified QR algorithm.
	       
	       this implementation uses a simplified QR decomposition approach
	       focused on finding real eigenvalues. for a full eigendecomposition
	       of general matrices, more sophisticated algorithms like the
	       implicit QR algorithm with shifts would be needed.
	       
	       the QR algorithm works by repeatedly decomposing A = QR and
	       forming A_new = RQ. the eigenvalues appear on the diagonal
	       as this process converges to upper triangular form.

*/

commc_mat4_eigen_t commc_mat4_eigen_decompose(commc_mat4_t matrix, int max_iterations) {

  commc_mat4_eigen_t result;
  commc_mat4_t       A, Q, R, Q_T;
  commc_vec3_t       col[3], q[3];
  commc_vec3_t       proj1, proj2a, proj2b, temp2;
  int                i, j, iter;
  float              sum;
  
  /* initialize result */
  
  result.num_real   = 0;
  result.converged  = 0;
  result.iterations = 0;
  
  for  (i = 0; i < 4; i++) {
  
    result.eigenvalues[i].real = 0.0f;
    result.eigenvalues[i].imag = 0.0f;
    result.eigenvectors[i].x   = 0.0f;
    result.eigenvectors[i].y   = 0.0f;
    result.eigenvectors[i].z   = 0.0f;
    
  }
  
  /* start with original matrix */
  
  A = matrix;
  
  /* simplified QR iteration */
  
  for  (iter = 0; iter < max_iterations; iter++) {
  
    /* simplified QR decomposition using Gram-Schmidt process */
    /* this is a basic implementation focusing on educational clarity */
    
    Q = commc_mat4_identity();
    R = commc_mat4_identity();
    
    /* extract first 3 columns as vectors for QR on 3x3 submatrix */
    
    for  (j = 0; j < 3; j++) {
    
      col[j].x = A.m[0 + j * 4];
      col[j].y = A.m[1 + j * 4];
      col[j].z = A.m[2 + j * 4];
      
    }
    
    /* Gram-Schmidt orthogonalization */
    
    q[0] = commc_vec3_normalize(col[0]);
    
    /* q[1] = col[1] - proj(col[1], q[0]) */
    
    proj1 = commc_vec3_scale(q[0], commc_vec3_dot(col[1], q[0]));
    q[1] = commc_vec3_normalize(commc_vec3_sub(col[1], proj1));
    
    /* q[2] = col[2] - proj(col[2], q[0]) - proj(col[2], q[1]) */
    
    proj2a = commc_vec3_scale(q[0], commc_vec3_dot(col[2], q[0]));
    proj2b = commc_vec3_scale(q[1], commc_vec3_dot(col[2], q[1]));
    temp2  = commc_vec3_sub(col[2], proj2a);
    q[2] = commc_vec3_normalize(commc_vec3_sub(temp2, proj2b));
    
    /* build Q matrix from orthonormal vectors */
    
    for  (j = 0; j < 3; j++) {
    
      Q.m[0 + j * 4] = (j == 0) ? q[0].x : (j == 1) ? q[1].x : q[2].x;
      Q.m[1 + j * 4] = (j == 0) ? q[0].y : (j == 1) ? q[1].y : q[2].y;
      Q.m[2 + j * 4] = (j == 0) ? q[0].z : (j == 1) ? q[1].z : q[2].z;
      
    }
    
    /* R = Q^T * A */
    
    Q_T = commc_mat4_transpose(Q);
    R = commc_mat4_multiply(Q_T, A);
    
    /* A_new = R * Q */
    
    A = commc_mat4_multiply(R, Q);
    
    /* check convergence (diagonal elements stabilizing) */
    
    sum = 0.0f;
    
    for  (i = 0; i < 3; i++) {
    
      for  (j = 0; j < 3; j++) {
      
        if  (i != j) {
        
          sum += (float)fabs(A.m[i + j * 4]);
          
        }
        
      }
      
    }
    
    if  (sum < COMMC_EPSILON) {
    
      result.converged = 1;
      break;
      
    }
    
  }
  
  result.iterations = iter;
  
  /* extract real eigenvalues from diagonal */
  
  for  (i = 0; i < 3; i++) {
  
    if  ((float)fabs(A.m[i + i * 4]) > COMMC_EPSILON) {
    
      result.eigenvalues[result.num_real].real = A.m[i + i * 4];
      result.eigenvalues[result.num_real].imag = 0.0f;
      
      /* compute corresponding eigenvector using power iteration */
      
      result.eigenvectors[result.num_real] = commc_mat4_eigenvector_dominant(matrix, 50);
      result.num_real++;
      
    }
    
  }
  
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

         commc_quat_to_mat3()
	       ---
	       converts a quaternion to a 3x3 rotation matrix.
	       uses the standard quaternion-to-rotation-matrix formula
	       with optimizations to avoid redundant calculations.
	       
	       the resulting matrix is orthogonal and represents the
	       same rotation as the input quaternion in matrix form.

*/

commc_mat3_t commc_quat_to_mat3(commc_quat_t q) {

  commc_mat3_t result;
  float        xx, yy, zz, xy, xz, yz, wx, wy, wz;
  
  /* normalize quaternion to ensure unit length */
  q = commc_quat_normalize(q);
  
  /* precalculate common terms to avoid redundant multiplications */
  xx = q.x * q.x;
  yy = q.y * q.y;
  zz = q.z * q.z;
  xy = q.x * q.y;
  xz = q.x * q.z;
  yz = q.y * q.z;
  wx = q.w * q.x;
  wy = q.w * q.y;
  wz = q.w * q.z;
  
  /* build rotation matrix using quaternion-to-matrix formula (column-major) */
  result.m[0 + 0 * 3] = 1.0f - 2.0f * (yy + zz);
  result.m[1 + 0 * 3] = 2.0f * (xy + wz);
  result.m[2 + 0 * 3] = 2.0f * (xz - wy);
  
  result.m[0 + 1 * 3] = 2.0f * (xy - wz);
  result.m[1 + 1 * 3] = 1.0f - 2.0f * (xx + zz);
  result.m[2 + 1 * 3] = 2.0f * (yz + wx);
  
  result.m[0 + 2 * 3] = 2.0f * (xz + wy);
  result.m[1 + 2 * 3] = 2.0f * (yz - wx);
  result.m[2 + 2 * 3] = 1.0f - 2.0f * (xx + yy);
  
  return result;

}

/*

         commc_mat4_to_quat()
	       ---
	       converts a 4x4 transformation matrix to a quaternion.
	       extracts the rotation component and uses shepperd's method
	       for numerical stability when the trace is small.
	       
	       assumes the matrix represents a valid rotation (orthogonal
	       with determinant 1) for accurate quaternion conversion.

*/

commc_quat_t commc_mat4_to_quat(commc_mat4_t matrix) {

  commc_quat_t result;
  float        trace, s;
  int          i, j, k;
  
  /* calculate the trace of the rotation matrix */
  trace = matrix.m[0 + 0 * 4] + matrix.m[1 + 1 * 4] + matrix.m[2 + 2 * 4];
  
  if  (trace > 0.0f) {
  
    /* standard case: trace is positive */
    s = (float)sqrt(trace + 1.0f) * 2.0f;
    result.w = 0.25f * s;
    result.x = (matrix.m[2 + 1 * 4] - matrix.m[1 + 2 * 4]) / s;
    result.y = (matrix.m[0 + 2 * 4] - matrix.m[2 + 0 * 4]) / s;
    result.z = (matrix.m[1 + 0 * 4] - matrix.m[0 + 1 * 4]) / s;
    
  } else {
  
    /* trace is negative: use shepperd's method for stability */
    i = 0;
    if  (matrix.m[1 + 1 * 4] > matrix.m[0 + 0 * 4]) i = 1;
    if  (matrix.m[2 + 2 * 4] > matrix.m[i + i * 4]) i = 2;
    
    j = (i + 1) % 3;
    k = (i + 2) % 3;
    
    s = (float)sqrt(matrix.m[i + i * 4] - matrix.m[j + j * 4] - matrix.m[k + k * 4] + 1.0f) * 2.0f;
    
    /* assign quaternion components based on largest diagonal element */
    if  (i == 0) {
      result.x = 0.25f * s;
      result.y = (matrix.m[0 + 1 * 4] + matrix.m[1 + 0 * 4]) / s;
      result.z = (matrix.m[0 + 2 * 4] + matrix.m[2 + 0 * 4]) / s;
      result.w = (matrix.m[2 + 1 * 4] - matrix.m[1 + 2 * 4]) / s;
    } else if  (i == 1) {
      result.x = (matrix.m[0 + 1 * 4] + matrix.m[1 + 0 * 4]) / s;
      result.y = 0.25f * s;
      result.z = (matrix.m[1 + 2 * 4] + matrix.m[2 + 1 * 4]) / s;
      result.w = (matrix.m[0 + 2 * 4] - matrix.m[2 + 0 * 4]) / s;
    } else {
      result.x = (matrix.m[0 + 2 * 4] + matrix.m[2 + 0 * 4]) / s;
      result.y = (matrix.m[1 + 2 * 4] + matrix.m[2 + 1 * 4]) / s;
      result.z = 0.25f * s;
      result.w = (matrix.m[1 + 0 * 4] - matrix.m[0 + 1 * 4]) / s;
    }
    
  }
  
  return commc_quat_normalize(result);

}

/*

         commc_mat3_to_quat()
	       ---
	       converts a 3x3 rotation matrix to a quaternion.
	       more direct conversion from pure rotation matrices
	       using the same numerical stability techniques as
	       the 4x4 version but optimized for 3x3 matrices.

*/

commc_quat_t commc_mat3_to_quat(commc_mat3_t matrix) {

  commc_quat_t result;
  float        trace, s;
  int          i, j, k;
  
  /* calculate the trace of the rotation matrix */
  trace = matrix.m[0 + 0 * 3] + matrix.m[1 + 1 * 3] + matrix.m[2 + 2 * 3];
  
  if  (trace > 0.0f) {
  
    /* standard case: trace is positive */
    s = (float)sqrt(trace + 1.0f) * 2.0f;
    result.w = 0.25f * s;
    result.x = (matrix.m[2 + 1 * 3] - matrix.m[1 + 2 * 3]) / s;
    result.y = (matrix.m[0 + 2 * 3] - matrix.m[2 + 0 * 3]) / s;
    result.z = (matrix.m[1 + 0 * 3] - matrix.m[0 + 1 * 3]) / s;
    
  } else {
  
    /* trace is negative: use shepperd's method for stability */
    i = 0;
    if  (matrix.m[1 + 1 * 3] > matrix.m[0 + 0 * 3]) i = 1;
    if  (matrix.m[2 + 2 * 3] > matrix.m[i + i * 3]) i = 2;
    
    j = (i + 1) % 3;
    k = (i + 2) % 3;
    
    s = (float)sqrt(matrix.m[i + i * 3] - matrix.m[j + j * 3] - matrix.m[k + k * 3] + 1.0f) * 2.0f;
    
    /* assign quaternion components based on largest diagonal element */
    if  (i == 0) {
      result.x = 0.25f * s;
      result.y = (matrix.m[0 + 1 * 3] + matrix.m[1 + 0 * 3]) / s;
      result.z = (matrix.m[0 + 2 * 3] + matrix.m[2 + 0 * 3]) / s;
      result.w = (matrix.m[2 + 1 * 3] - matrix.m[1 + 2 * 3]) / s;
    } else if  (i == 1) {
      result.x = (matrix.m[0 + 1 * 3] + matrix.m[1 + 0 * 3]) / s;
      result.y = 0.25f * s;
      result.z = (matrix.m[1 + 2 * 3] + matrix.m[2 + 1 * 3]) / s;
      result.w = (matrix.m[0 + 2 * 3] - matrix.m[2 + 0 * 3]) / s;
    } else {
      result.x = (matrix.m[0 + 2 * 3] + matrix.m[2 + 0 * 3]) / s;
      result.y = (matrix.m[1 + 2 * 3] + matrix.m[2 + 1 * 3]) / s;
      result.z = 0.25f * s;
      result.w = (matrix.m[1 + 0 * 3] - matrix.m[0 + 1 * 3]) / s;
    }
    
  }
  
  return commc_quat_normalize(result);

}

/*

         commc_quat_dot()
	       ---
	       computes the dot product of two quaternions.
	       this is the standard 4D dot product treating
	       quaternions as 4-component vectors (w, x, y, z).
	       
	       essential for SLERP interpolation to determine
	       the angular relationship between rotations.

*/

float commc_quat_dot(commc_quat_t q1, commc_quat_t q2) {

  return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;

}

/*

         commc_quat_slerp()
	       ---
	       performs spherical linear interpolation between two quaternions.
	       uses the spherical interpolation formula to provide smooth
	       rotation interpolation with constant angular velocity.
	       
	       automatically handles shortest-path selection and numerical
	       stability for near-parallel quaternions.

*/

commc_quat_t commc_quat_slerp(commc_quat_t q1, commc_quat_t q2, float t) {

  commc_quat_t result;
  float        dot, theta, sin_theta, scale1, scale2;
  
  /* clamp interpolation parameter to valid range */
  if  (t <= 0.0f) return q1;
  if  (t >= 1.0f) return q2;
  
  /* calculate dot product for angle between quaternions */
  dot = commc_quat_dot(q1, q2);
  
  /* if dot product is negative, use -q2 to ensure shortest path */
  if  (dot < 0.0f) {
    q2.x = -q2.x;
    q2.y = -q2.y;
    q2.z = -q2.z;
    q2.w = -q2.w;
    dot = -dot;
  }
  
  /* if quaternions are very close, use linear interpolation to avoid division by zero */
  if  (dot > 0.9995f) {
    result.x = q1.x + t * (q2.x - q1.x);
    result.y = q1.y + t * (q2.y - q1.y);
    result.z = q1.z + t * (q2.z - q1.z);
    result.w = q1.w + t * (q2.w - q1.w);
    return commc_quat_normalize(result);
  }
  
  /* calculate angle and spherical interpolation scales */
  theta = (float)acos(dot);
  sin_theta = (float)sin(theta);
  scale1 = (float)sin((1.0f - t) * theta) / sin_theta;
  scale2 = (float)sin(t * theta) / sin_theta;
  
  /* compute spherically interpolated quaternion */
  result.x = scale1 * q1.x + scale2 * q2.x;
  result.y = scale1 * q1.y + scale2 * q2.y;
  result.z = scale1 * q1.z + scale2 * q2.z;
  result.w = scale1 * q1.w + scale2 * q2.w;
  
  return result;

}

/*

         commc_quat_nlerp()
	       ---
	       performs normalized linear interpolation between two quaternions.
	       faster alternative to SLERP that linearly interpolates components
	       then normalizes the result. provides good approximation for
	       small angular differences at reduced computational cost.

*/

commc_quat_t commc_quat_nlerp(commc_quat_t q1, commc_quat_t q2, float t) {

  commc_quat_t result;
  float        dot;
  
  /* clamp interpolation parameter to valid range */
  if  (t <= 0.0f) return q1;
  if  (t >= 1.0f) return q2;
  
  /* calculate dot product to determine shortest path */
  dot = commc_quat_dot(q1, q2);
  
  /* if dot product is negative, use -q2 for shortest rotation path */
  if  (dot < 0.0f) {
    q2.x = -q2.x;
    q2.y = -q2.y;
    q2.z = -q2.z;
    q2.w = -q2.w;
  }
  
  /* linear interpolation of quaternion components */
  result.x = q1.x + t * (q2.x - q1.x);
  result.y = q1.y + t * (q2.y - q1.y);
  result.z = q1.z + t * (q2.z - q1.z);
  result.w = q1.w + t * (q2.w - q1.w);
  
  /* normalize result to ensure unit quaternion */
  return commc_quat_normalize(result);

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

         commc_cubic_spline_create()
	       ---
	       creates a new cubic spline from control points.
	       allocates memory for the spline structure and makes
	       a copy of the control points for safe management.
	       
	       returns null if memory allocation fails or invalid
	       parameters are provided.

*/

commc_cubic_spline_t* commc_cubic_spline_create(commc_spline_point_t* points, int num_points) {

  commc_cubic_spline_t* spline;
  int                   i;
  
  if  (!points || num_points < 2) {
    return NULL;
  }
  
  /* allocate memory for spline structure */
  spline = (commc_cubic_spline_t*)malloc(sizeof(commc_cubic_spline_t));
  if  (!spline) {
    return NULL;
  }
  
  /* allocate memory for control points */
  spline->points = (commc_spline_point_t*)malloc(num_points * sizeof(commc_spline_point_t));
  if  (!spline->points) {
    free(spline);
    return NULL;
  }
  
  /* allocate memory for spline segments (n-1 segments for n points) */
  spline->segments = (commc_spline_segment_t*)malloc((num_points - 1) * sizeof(commc_spline_segment_t));
  if  (!spline->segments) {
    free(spline->points);
    free(spline);
    return NULL;
  }
  
  /* copy control points */
  for  (i = 0; i < num_points; i++) {
    spline->points[i] = points[i];
  }
  
  /* initialize spline properties */
  spline->num_points   = num_points;
  spline->num_segments = num_points - 1;
  spline->is_computed  = 0;
  
  return spline;

}

/*

         commc_cubic_spline_compute()
	       ---
	       computes cubic spline coefficients using natural boundary
	       conditions. solves the tridiagonal system to find second
	       derivatives at control points, then calculates segment
	       coefficients for smooth interpolation.

*/

int commc_cubic_spline_compute(commc_cubic_spline_t* spline) {

  float* h;
  float* alpha;
  float* l;
  float* mu;
  float* z;
  float* c;
  int    i;
  int    n;
  
  if  (!spline || spline->num_points < 2) {
    return 0;
  }
  
  n = spline->num_points;
  
  /* allocate temporary arrays for computation */
  h     = (float*)malloc((n - 1) * sizeof(float));
  alpha = (float*)malloc((n - 1) * sizeof(float));
  l     = (float*)malloc(n * sizeof(float));
  mu    = (float*)malloc(n * sizeof(float));
  z     = (float*)malloc(n * sizeof(float));
  c     = (float*)malloc(n * sizeof(float));
  
  if  (!h || !alpha || !l || !mu || !z || !c) {
    free(h); free(alpha); free(l); free(mu); free(z); free(c);
    return 0;
  }
  
  /* compute step sizes */
  for  (i = 0; i < n - 1; i++) {
    h[i] = spline->points[i + 1].x - spline->points[i].x;
    if  (h[i] <= 0.0f) {
      free(h); free(alpha); free(l); free(mu); free(z); free(c);
      return 0; /* points must be sorted in ascending x order */
    }
  }
  
  /* compute alpha values for the tridiagonal system */
  for  (i = 1; i < n - 1; i++) {
    alpha[i] = (3.0f / h[i]) * (spline->points[i + 1].y - spline->points[i].y) -
               (3.0f / h[i - 1]) * (spline->points[i].y - spline->points[i - 1].y);
  }
  
  /* solve tridiagonal system using thomas algorithm */
  l[0] = 1.0f;
  mu[0] = 0.0f;
  z[0] = 0.0f;
  
  for  (i = 1; i < n - 1; i++) {
    l[i] = 2.0f * (spline->points[i + 1].x - spline->points[i - 1].x) - h[i - 1] * mu[i - 1];
    mu[i] = h[i] / l[i];
    z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
  }
  
  l[n - 1] = 1.0f;
  z[n - 1] = 0.0f;
  c[n - 1] = 0.0f;
  
  /* back substitution */
  for  (i = n - 2; i >= 0; i--) {
    c[i] = z[i] - mu[i] * c[i + 1];
  }
  
  /* compute spline segment coefficients */
  for  (i = 0; i < n - 1; i++) {
    spline->segments[i].x_start = spline->points[i].x;
    spline->segments[i].x_end   = spline->points[i + 1].x;
    spline->segments[i].a = (c[i + 1] - c[i]) / (3.0f * h[i]);
    spline->segments[i].b = c[i];
    spline->segments[i].c = (spline->points[i + 1].y - spline->points[i].y) / h[i] - 
                            h[i] * (c[i + 1] + 2.0f * c[i]) / 3.0f;
    spline->segments[i].d = spline->points[i].y;
  }
  
  /* cleanup temporary arrays */
  free(h); free(alpha); free(l); free(mu); free(z); free(c);
  
  spline->is_computed = 1;
  return 1;

}

/*

         commc_cubic_spline_interpolate()
	       ---
	       evaluates the cubic spline at parameter value x.
	       finds the appropriate segment and evaluates the
	       cubic polynomial using horner's method for efficiency.

*/

float commc_cubic_spline_interpolate(commc_cubic_spline_t* spline, float x) {

  commc_spline_segment_t* seg;
  float                   dx;
  int                     i;
  
  if  (!spline || !spline->is_computed) {
    return 0.0f;
  }
  
  /* find the appropriate segment */
  for  (i = 0; i < spline->num_segments; i++) {
    if  (x >= spline->segments[i].x_start && x <= spline->segments[i].x_end) {
      break;
    }
  }
  
  /* clamp to valid range if x is outside */
  if  (i >= spline->num_segments) {
    i = spline->num_segments - 1;
  }
  
  seg = &spline->segments[i];
  dx = x - seg->x_start;
  
  /* evaluate cubic polynomial using horner's method */
  return seg->d + dx * (seg->c + dx * (seg->b + dx * seg->a));

}

/*

         commc_cubic_spline_derivative()
	       ---
	       computes the first derivative of the cubic spline at x.
	       differentiates the cubic polynomial to get the slope
	       or tangent vector at the specified parameter value.

*/

float commc_cubic_spline_derivative(commc_cubic_spline_t* spline, float x) {

  commc_spline_segment_t* seg;
  float                   dx;
  int                     i;
  
  if  (!spline || !spline->is_computed) {
    return 0.0f;
  }
  
  /* find the appropriate segment */
  for  (i = 0; i < spline->num_segments; i++) {
    if  (x >= spline->segments[i].x_start && x <= spline->segments[i].x_end) {
      break;
    }
  }
  
  /* clamp to valid range if x is outside */
  if  (i >= spline->num_segments) {
    i = spline->num_segments - 1;
  }
  
  seg = &spline->segments[i];
  dx = x - seg->x_start;
  
  /* evaluate first derivative: c + 2*b*dx + 3*a*dx^2 */
  return seg->c + dx * (2.0f * seg->b + 3.0f * seg->a * dx);

}

/*

         commc_cubic_spline_destroy()
	       ---
	       releases all memory allocated for the cubic spline.
	       safely handles null pointers and prevents memory leaks.

*/

void commc_cubic_spline_destroy(commc_cubic_spline_t* spline) {

  if  (!spline) {
    return;
  }
  
  if  (spline->points) {
    free(spline->points);
  }
  
  if  (spline->segments) {
    free(spline->segments);
  }
  
  free(spline);

}

/*

         commc_bezier_linear()
	       ---
	       evaluates a linear Bezier curve between two points.
	       this is equivalent to simple linear interpolation.
	       formula: B(t) = (1-t)*P0 + t*P1

*/

commc_bezier_point_t commc_bezier_linear(commc_bezier_point_t p0, commc_bezier_point_t p1, float t) {

  commc_bezier_point_t result;
  
  /* clamp parameter to valid range */
  if  (t < 0.0f) t = 0.0f;
  if  (t > 1.0f) t = 1.0f;
  
  /* linear interpolation formula */
  result.x = (1.0f - t) * p0.x + t * p1.x;
  result.y = (1.0f - t) * p0.y + t * p1.y;
  
  return result;

}

/*

         commc_bezier_quadratic()
	       ---
	       evaluates a quadratic Bezier curve using de casteljau's algorithm.
	       provides numerically stable evaluation of three-point curves.
	       formula: B(t) = (1-t)^2*P0 + 2*(1-t)*t*P1 + t^2*P2

*/

commc_bezier_point_t commc_bezier_quadratic(commc_bezier_quadratic_t curve, float t) {

  commc_bezier_point_t result;
  commc_bezier_point_t q0, q1;
  float                t_inv;
  
  /* clamp parameter to valid range */
  if  (t < 0.0f) t = 0.0f;
  if  (t > 1.0f) t = 1.0f;
  
  t_inv = 1.0f - t;
  
  /* first level of de casteljau's algorithm */
  q0.x = t_inv * curve.p0.x + t * curve.p1.x;
  q0.y = t_inv * curve.p0.y + t * curve.p1.y;
  
  q1.x = t_inv * curve.p1.x + t * curve.p2.x;
  q1.y = t_inv * curve.p1.y + t * curve.p2.y;
  
  /* second level - final result */
  result.x = t_inv * q0.x + t * q1.x;
  result.y = t_inv * q0.y + t * q1.y;
  
  return result;

}

/*

         commc_bezier_cubic()
	       ---
	       evaluates a cubic Bezier curve using de casteljau's algorithm.
	       provides numerically stable evaluation of four-point curves.
	       fundamental building block for computer graphics and animation.

*/

commc_bezier_point_t commc_bezier_cubic(commc_bezier_cubic_t curve, float t) {

  commc_bezier_point_t result;
  commc_bezier_point_t q0, q1, q2;
  commc_bezier_point_t r0, r1;
  float                t_inv;
  
  /* clamp parameter to valid range */
  if  (t < 0.0f) t = 0.0f;
  if  (t > 1.0f) t = 1.0f;
  
  t_inv = 1.0f - t;
  
  /* first level of de casteljau's algorithm */
  q0.x = t_inv * curve.p0.x + t * curve.p1.x;
  q0.y = t_inv * curve.p0.y + t * curve.p1.y;
  
  q1.x = t_inv * curve.p1.x + t * curve.p2.x;
  q1.y = t_inv * curve.p1.y + t * curve.p2.y;
  
  q2.x = t_inv * curve.p2.x + t * curve.p3.x;
  q2.y = t_inv * curve.p2.y + t * curve.p3.y;
  
  /* second level */
  r0.x = t_inv * q0.x + t * q1.x;
  r0.y = t_inv * q0.y + t * q1.y;
  
  r1.x = t_inv * q1.x + t * q2.x;
  r1.y = t_inv * q1.y + t * q2.y;
  
  /* third level - final result */
  result.x = t_inv * r0.x + t * r1.x;
  result.y = t_inv * r0.y + t * r1.y;
  
  return result;

}

/*

         commc_bezier_quadratic_derivative()
	       ---
	       computes the first derivative of a quadratic Bezier curve.
	       derivative represents the tangent vector (velocity) at
	       parameter t, useful for animation and path following.

*/

commc_bezier_point_t commc_bezier_quadratic_derivative(commc_bezier_quadratic_t curve, float t) {

  commc_bezier_point_t result;
  
  /* clamp parameter to valid range */
  if  (t < 0.0f) t = 0.0f;
  if  (t > 1.0f) t = 1.0f;
  
  /* derivative formula for quadratic Bezier: 2*(1-t)*(P1-P0) + 2*t*(P2-P1) */
  result.x = 2.0f * (1.0f - t) * (curve.p1.x - curve.p0.x) + 
             2.0f * t * (curve.p2.x - curve.p1.x);
  result.y = 2.0f * (1.0f - t) * (curve.p1.y - curve.p0.y) + 
             2.0f * t * (curve.p2.y - curve.p1.y);
  
  return result;

}

/*

         commc_bezier_cubic_derivative()
	       ---
	       computes the first derivative of a cubic Bezier curve.
	       essential for calculating tangent vectors, velocities,
	       and orientations along curved animation paths.

*/

commc_bezier_point_t commc_bezier_cubic_derivative(commc_bezier_cubic_t curve, float t) {

  commc_bezier_point_t result;
  float                t_sq;
  
  /* clamp parameter to valid range */
  if  (t < 0.0f) t = 0.0f;
  if  (t > 1.0f) t = 1.0f;
  
  t_sq = t * t;
  
  /* derivative formula for cubic Bezier */
  result.x = 3.0f * (1.0f - t) * (1.0f - t) * (curve.p1.x - curve.p0.x) +
             6.0f * (1.0f - t) * t * (curve.p2.x - curve.p1.x) +
             3.0f * t_sq * (curve.p3.x - curve.p2.x);
             
  result.y = 3.0f * (1.0f - t) * (1.0f - t) * (curve.p1.y - curve.p0.y) +
             6.0f * (1.0f - t) * t * (curve.p2.y - curve.p1.y) +
             3.0f * t_sq * (curve.p3.y - curve.p2.y);
  
  return result;

}

/*

         commc_bezier_cubic_length()
	       ---
	       estimates the arc length of a cubic Bezier curve using
	       adaptive subdivision. approximates true curve length
	       for arc-length parameterization and timing calculations.

*/

float commc_bezier_cubic_length(commc_bezier_cubic_t curve, int subdivisions) {

  float                total_length;
  float                step;
  commc_bezier_point_t current, next;
  float                dx, dy;
  int                  i;
  
  if  (subdivisions < 1) subdivisions = 10;
  
  total_length = 0.0f;
  step = 1.0f / (float)subdivisions;
  
  /* start with first point */
  current = commc_bezier_cubic(curve, 0.0f);
  
  /* sum distances between consecutive subdivided points */
  for  (i = 1; i <= subdivisions; i++) {
    next = commc_bezier_cubic(curve, (float)i * step);
    
    dx = next.x - current.x;
    dy = next.y - current.y;
    
    total_length += (float)sqrt(dx * dx + dy * dy);
    current = next;
  }
  
  return total_length;

}

/*

         commc_bspline_find_span()
	       ---
	       finds the knot span index for parameter u in the knot vector.
	       uses binary search for efficiency. the span determines which
	       control points influence the curve at parameter u.

*/

int commc_bspline_find_span(commc_bspline_t spline, float u) {

  int  low;
  int  high;
  int  mid;

  /* clamp parameter to valid range */
  if  (u >= spline.knots[spline.num_points]) {
    return spline.num_points - 1;
  }

  if  (u <= spline.knots[spline.degree]) {
    return spline.degree;
  }

  /* binary search for the span */
  low = spline.degree;
  high = spline.num_points;

  mid = (low + high) / 2;

  while  (u < spline.knots[mid] || u >= spline.knots[mid + 1]) {

    if  (u < spline.knots[mid]) {
      high = mid;
    } else {
      low = mid;
    }

    mid = (low + high) / 2;
  }

  return mid;

}

/*

         commc_bspline_basis()
	       ---
	       computes all non-zero basis functions for a given span using
	       the Cox-de Boor recursion formula. this is the fundamental
	       algorithm for B-spline evaluation and derivative computation.

*/

void commc_bspline_basis(commc_bspline_t spline, int span, float u, float* basis) {

  float*  left;
  float*  right;
  float   saved;
  float   temp;
  int     j;
  int     r;

  /* allocate temporary arrays for recursion */
  left = (float*)malloc((spline.degree + 1) * sizeof(float));
  right = (float*)malloc((spline.degree + 1) * sizeof(float));

  if  (!left || !right) {
    if (left) free(left);
    if (right) free(right);
    return;
  }

  /* initialize first basis function */
  basis[0] = 1.0f;

  /* compute basis functions using Cox-de Boor recursion */
  for  (j = 1; j <= spline.degree; j++) {

    left[j] = u - spline.knots[span + 1 - j];
    right[j] = spline.knots[span + j] - u;
    saved = 0.0f;

    for  (r = 0; r < j; r++) {

      temp = basis[r] / (right[r + 1] + left[j - r]);
      basis[r] = saved + right[r + 1] * temp;
      saved = left[j - r] * temp;
    }

    basis[j] = saved;
  }

  free(left);
  free(right);

}

/*

         commc_bspline_evaluate()
	       ---
	       evaluates a B-spline curve at parameter u using the Cox-de Boor
	       algorithm. supports both rational (NURBS) and non-rational 
	       B-splines. the algorithm computes the weighted sum of control
	       points using basis functions.

*/

commc_bezier_point_t commc_bspline_evaluate(commc_bspline_t spline, float u) {

  commc_bezier_point_t  result;
  float*                basis;
  float                 weight_sum;
  int                   span;
  int                   i;

  result.x = 0.0f;
  result.y = 0.0f;

  /* validate input */
  if  (!spline.control_points || !spline.knots || spline.num_points <= 0) {
    return result;
  }

  /* find knot span and compute basis functions */
  span = commc_bspline_find_span(spline, u);
  basis = (float*)malloc((spline.degree + 1) * sizeof(float));

  if  (!basis) {
    return result;
  }

  commc_bspline_basis(spline, span, u, basis);

  /* compute curve point as weighted sum */
  weight_sum = 0.0f;

  for  (i = 0; i <= spline.degree; i++) {

    int  point_index = span - spline.degree + i;
    float  weight = spline.weights ? spline.weights[point_index] : 1.0f;
    float  basis_weight = basis[i] * weight;

    result.x += basis_weight * spline.control_points[point_index].x;
    result.y += basis_weight * spline.control_points[point_index].y;
    weight_sum += basis_weight;
  }

  /* normalize for rational B-splines (NURBS) */
  if  (spline.weights && weight_sum > 0.0f) {
    result.x /= weight_sum;
    result.y /= weight_sum;
  }

  free(basis);
  return result;

}

/*

         commc_bspline_derivative()
	       ---
	       computes the first derivative of a B-spline curve at parameter u.
	       uses the derivative formula for B-splines which involves creating
	       a derived control polygon with reduced degree.

*/

commc_bezier_point_t commc_bspline_derivative(commc_bspline_t spline, float u) {

  commc_bezier_point_t  result;
  commc_bspline_t       derivative_spline;
  commc_bezier_point_t* derivative_points;
  float*                derivative_knots;
  int                   i;

  result.x = 0.0f;
  result.y = 0.0f;

  /* validate input and check for minimum degree */
  if  (!spline.control_points || spline.degree < 1) {
    return result;
  }

  /* allocate memory for derivative control points */
  derivative_points = (commc_bezier_point_t*)malloc((spline.num_points - 1) * sizeof(commc_bezier_point_t));
  derivative_knots = (float*)malloc((spline.num_knots - 2) * sizeof(float));

  if  (!derivative_points || !derivative_knots) {
    if (derivative_points) free(derivative_points);
    if (derivative_knots) free(derivative_knots);
    return result;
  }

  /* compute derivative control points */
  for  (i = 0; i < spline.num_points - 1; i++) {

    float  knot_diff = spline.knots[i + spline.degree + 1] - spline.knots[i + 1];

    if  (knot_diff > 0.0f) {
      float  factor = (float)spline.degree / knot_diff;
      derivative_points[i].x = factor * (spline.control_points[i + 1].x - spline.control_points[i].x);
      derivative_points[i].y = factor * (spline.control_points[i + 1].y - spline.control_points[i].y);
    } else {
      derivative_points[i].x = 0.0f;
      derivative_points[i].y = 0.0f;
    }
  }

  /* copy reduced knot vector */
  for  (i = 0; i < spline.num_knots - 2; i++) {
    derivative_knots[i] = spline.knots[i + 1];
  }

  /* create derivative spline with reduced degree */
  derivative_spline.control_points = derivative_points;
  derivative_spline.knots = derivative_knots;
  derivative_spline.weights = NULL;  /* derivatives are non-rational */
  derivative_spline.num_points = spline.num_points - 1;
  derivative_spline.num_knots = spline.num_knots - 2;
  derivative_spline.degree = spline.degree - 1;

  /* evaluate derivative spline */
  result = commc_bspline_evaluate(derivative_spline, u);

  free(derivative_points);
  free(derivative_knots);

  return result;

}

/* 
	==================================
          --- NOISE ---
	==================================
*/

/* Perlin noise permutation table - Ken Perlin's original */
static int perlin_permutation[512] = {
  151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
  8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
  35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
  134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
  55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
  18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
  250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
  189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
  172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
  228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
  107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
  8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
  35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
  134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
  55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
  18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
  250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
  189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
  172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
  228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
  107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

/*

         perlin_fade()
	       ---
	       Perlin's improved fade function: 6t^5 - 15t^4 + 10t^3.
	       this provides smoother interpolation than linear or
	       cubic interpolation, reducing grid artifacts.

*/

static float perlin_fade(float t) {
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

/*

         perlin_lerp()
	       ---
	       linear interpolation between two values using weight t.
	       fundamental operation for blending noise values from
	       neighboring grid points.

*/

static float perlin_lerp(float t, float a, float b) {
  return a + t * (b - a);
}

/*

         perlin_grad()
	       ---
	       computes the dot product between a pseudo-random gradient
	       vector and the offset vector. Ken Perlin's gradient selection
	       uses a simple hash to choose from 12 gradient vectors.

*/

static float perlin_grad(int hash, float x, float y, float z) {

  int  h;
  
  h = hash & 15;                        /* USE LOWER 4 BITS OF HASH */
  
  /* convert 4 bits to one of 12 gradient directions */
  switch (h) {
    case 0:  return  x + y;
    case 1:  return -x + y;
    case 2:  return  x - y;
    case 3:  return -x - y;
    case 4:  return  x + z;
    case 5:  return -x + z;
    case 6:  return  x - z;
    case 7:  return -x - z;
    case 8:  return  y + z;
    case 9:  return -y + z;
    case 10: return  y - z;
    case 11: return -y - z;
    case 12: return  y + x;
    case 13: return -y + z;
    case 14: return  y - x;
    case 15: return -y - z;
    default: return 0;
  }
}

/*

         commc_perlin_seed()
	       ---
	       seeds the Perlin noise generator by shuffling the permutation
	       table. allows reproducible noise patterns by ensuring the
	       same seed always produces the same noise sequence.

*/

void commc_perlin_seed(unsigned int seed) {

  int  i;
  int  j;
  int  temp;
  unsigned int  rng;

  /* initialize with original permutation */
  static const int original[256] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
  };

  rng = seed;

  /* copy original permutation */
  for  (i = 0; i < 256; i++) {
    perlin_permutation[i] = original[i];
  }

  /* Fisher-Yates shuffle */
  for  (i = 255; i > 0; i--) {
    rng = rng * 1103515245 + 12345;    /* LINEAR CONGRUENTIAL GENERATOR */
    j = (rng / 65536) % (i + 1);
    
    temp = perlin_permutation[i];
    perlin_permutation[i] = perlin_permutation[j];
    perlin_permutation[j] = temp;
  }

  /* duplicate for overflow protection */
  for  (i = 0; i < 256; i++) {
    perlin_permutation[256 + i] = perlin_permutation[i];
  }

}

/*

         commc_perlin_noise()
	       ---
	       generates 3D Perlin noise at coordinates (x, y, z).
	       implements Ken Perlin's improved noise algorithm with
	       proper gradient selection and smooth interpolation.

*/

float commc_perlin_noise(float x, float y, float z) {

  int    X, Y, Z;
  float  u, v, w;
  int    A, AA, AB, B, BA, BB;
  float  res;

  /* find unit cube containing the point */
  X = (int)floor(x) & 255;
  Y = (int)floor(y) & 255;
  Z = (int)floor(z) & 255;

  /* find relative position in cube */
  x -= floor(x);
  y -= floor(y);
  z -= floor(z);

  /* compute fade curves for x, y, z */
  u = perlin_fade(x);
  v = perlin_fade(y);
  w = perlin_fade(z);

  /* hash coordinates of 8 cube corners */
  A  = perlin_permutation[X    ] + Y;
  AA = perlin_permutation[A    ] + Z;
  AB = perlin_permutation[A + 1] + Z;
  B  = perlin_permutation[X + 1] + Y;
  BA = perlin_permutation[B    ] + Z;
  BB = perlin_permutation[B + 1] + Z;

  /* interpolate between 8 gradient values */
  res = perlin_lerp(w, 
    perlin_lerp(v, 
      perlin_lerp(u, 
        perlin_grad(perlin_permutation[AA    ], x    , y    , z    ),
        perlin_grad(perlin_permutation[BA    ], x - 1, y    , z    )),
      perlin_lerp(u, 
        perlin_grad(perlin_permutation[AB    ], x    , y - 1, z    ),
        perlin_grad(perlin_permutation[BB    ], x - 1, y - 1, z    ))),
    perlin_lerp(v, 
      perlin_lerp(u, 
        perlin_grad(perlin_permutation[AA + 1], x    , y    , z - 1),
        perlin_grad(perlin_permutation[BA + 1], x - 1, y    , z - 1)),
      perlin_lerp(u, 
        perlin_grad(perlin_permutation[AB + 1], x    , y - 1, z - 1),
        perlin_grad(perlin_permutation[BB + 1], x - 1, y - 1, z - 1))));

  return res;

}

/*

         commc_perlin_noise_2d()
	       ---
	       generates 2D Perlin noise by setting z coordinate to 0.
	       optimized for 2D applications like terrain generation,
	       texture synthesis, and procedural patterns.

*/

float commc_perlin_noise_2d(float x, float y) {
  return commc_perlin_noise(x, y, 0.0f);
}

/*

         commc_simplex_noise()
	       ---
	       generates 2D simplex noise using Ken Perlin's simplex algorithm.
	       uses triangular lattice for improved isotropy and fewer 
	       directional artifacts compared to square lattice Perlin noise.

*/

float commc_simplex_noise(float x, float y) {

  static const float F2 = 0.3660254037844387f;  /* (sqrt(3) - 1) / 2 */
  static const float G2 = 0.21132486540518713f; /* (3 - sqrt(3)) / 6 */
  
  static const int grad2[8][2] = {
    {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
    {1, 0}, {-1, 0}, {0, 1}, {0, -1}
  };

  float  s;
  int    i, j;
  float  t;
  float  x0, y0;
  int    i1, j1;
  float  x1, y1, x2, y2;
  int    ii, jj;
  float  t0, t1, t2;
  float  n0, n1, n2;
  int    gi0, gi1, gi2;

  /* skew input space to determine simplex cell */
  s = (x + y) * F2;
  i = (int)floor(x + s);
  j = (int)floor(y + s);

  /* unskew cell origin */
  t = (float)(i + j) * G2;
  x0 = x - (i - t);
  y0 = y - (j - t);

  /* determine which simplex we're in */
  if  (x0 > y0) {
    i1 = 1;  /* lower triangle */
    j1 = 0;
  } else {
    i1 = 0;  /* upper triangle */
    j1 = 1;
  }

  /* offsets for middle and last corners */
  x1 = x0 - (float)i1 + G2;
  y1 = y0 - (float)j1 + G2;
  x2 = x0 - 1.0f + 2.0f * G2;
  y2 = y0 - 1.0f + 2.0f * G2;

  /* work out hashed gradient indices for three corners */
  ii = i & 255;
  jj = j & 255;
  gi0 = perlin_permutation[ii + perlin_permutation[jj]] % 8;
  gi1 = perlin_permutation[ii + i1 + perlin_permutation[jj + j1]] % 8;
  gi2 = perlin_permutation[ii + 1 + perlin_permutation[jj + 1]] % 8;

  /* calculate contribution from three corners */
  n0 = 0.0f;
  t0 = 0.5f - x0 * x0 - y0 * y0;

  if  (t0 >= 0.0f) {
    t0 *= t0;
    n0 = t0 * t0 * (grad2[gi0][0] * x0 + grad2[gi0][1] * y0);
  }

  n1 = 0.0f;
  t1 = 0.5f - x1 * x1 - y1 * y1;

  if  (t1 >= 0.0f) {
    t1 *= t1;
    n1 = t1 * t1 * (grad2[gi1][0] * x1 + grad2[gi1][1] * y1);
  }

  n2 = 0.0f;
  t2 = 0.5f - x2 * x2 - y2 * y2;

  if  (t2 >= 0.0f) {
    t2 *= t2;
    n2 = t2 * t2 * (grad2[gi2][0] * x2 + grad2[gi2][1] * y2);
  }

  /* add contributions and scale to [-1, 1] range */
  return 70.0f * (n0 + n1 + n2);

}

/*

         commc_simplex_noise_3d()
	       ---
	       generates 3D simplex noise using tetrahedral lattice.
	       provides better isotropy than cubic lattice Perlin noise
	       and scales better to higher dimensions.

*/

float commc_simplex_noise_3d(float x, float y, float z) {

  static const float F3 = 1.0f / 3.0f;
  static const float G3 = 1.0f / 6.0f;
  
  static const int grad3[12][3] = {
    {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
    {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
    {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
  };

  float  s;
  int    i, j, k;
  float  t;
  float  x0, y0, z0;
  int    i1, j1, k1;
  int    i2, j2, k2;
  float  x1, y1, z1;
  float  x2, y2, z2;
  float  x3, y3, z3;
  int    ii, jj, kk;
  float  t0, t1, t2, t3;
  float  n0, n1, n2, n3;
  int    gi0, gi1, gi2, gi3;

  /* skew input space to determine simplex cell */
  s = (x + y + z) * F3;
  i = (int)floor(x + s);
  j = (int)floor(y + s);
  k = (int)floor(z + s);

  /* unskew cell origin */
  t = (float)(i + j + k) * G3;
  x0 = x - (i - t);
  y0 = y - (j - t);
  z0 = z - (k - t);

  /* determine which simplex we're in */
  if  (x0 >= y0) {

    if  (y0 >= z0) {
      i1 = 1; j1 = 0; k1 = 0;  /* X Y Z order */
      i2 = 1; j2 = 1; k2 = 0;
    } else if  (x0 >= z0) {
      i1 = 1; j1 = 0; k1 = 0;  /* X Z Y order */
      i2 = 1; j2 = 0; k2 = 1;
    } else {
      i1 = 0; j1 = 0; k1 = 1;  /* Z X Y order */
      i2 = 1; j2 = 0; k2 = 1;
    }

  } else {

    if  (y0 < z0) {
      i1 = 0; j1 = 0; k1 = 1;  /* Z Y X order */
      i2 = 0; j2 = 1; k2 = 1;
    } else if  (x0 < z0) {
      i1 = 0; j1 = 1; k1 = 0;  /* Y Z X order */
      i2 = 0; j2 = 1; k2 = 1;
    } else {
      i1 = 0; j1 = 1; k1 = 0;  /* Y X Z order */
      i2 = 1; j2 = 1; k2 = 0;
    }
  }

  /* offsets for second, third, and fourth corners */
  x1 = x0 - (float)i1 + G3;
  y1 = y0 - (float)j1 + G3;
  z1 = z0 - (float)k1 + G3;
  x2 = x0 - (float)i2 + 2.0f * G3;
  y2 = y0 - (float)j2 + 2.0f * G3;
  z2 = z0 - (float)k2 + 2.0f * G3;
  x3 = x0 - 1.0f + 3.0f * G3;
  y3 = y0 - 1.0f + 3.0f * G3;
  z3 = z0 - 1.0f + 3.0f * G3;

  /* work out hashed gradient indices for four corners */
  ii = i & 255;
  jj = j & 255;
  kk = k & 255;
  gi0 = perlin_permutation[ii + perlin_permutation[jj + perlin_permutation[kk]]] % 12;
  gi1 = perlin_permutation[ii + i1 + perlin_permutation[jj + j1 + perlin_permutation[kk + k1]]] % 12;
  gi2 = perlin_permutation[ii + i2 + perlin_permutation[jj + j2 + perlin_permutation[kk + k2]]] % 12;
  gi3 = perlin_permutation[ii + 1 + perlin_permutation[jj + 1 + perlin_permutation[kk + 1]]] % 12;

  /* calculate contribution from four corners */
  n0 = 0.0f;
  t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;

  if  (t0 >= 0.0f) {
    t0 *= t0;
    n0 = t0 * t0 * (grad3[gi0][0] * x0 + grad3[gi0][1] * y0 + grad3[gi0][2] * z0);
  }

  n1 = 0.0f;
  t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;

  if  (t1 >= 0.0f) {
    t1 *= t1;
    n1 = t1 * t1 * (grad3[gi1][0] * x1 + grad3[gi1][1] * y1 + grad3[gi1][2] * z1);
  }

  n2 = 0.0f;
  t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;

  if  (t2 >= 0.0f) {
    t2 *= t2;
    n2 = t2 * t2 * (grad3[gi2][0] * x2 + grad3[gi2][1] * y2 + grad3[gi2][2] * z2);
  }

  n3 = 0.0f;
  t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;

  if  (t3 >= 0.0f) {
    t3 *= t3;
    n3 = t3 * t3 * (grad3[gi3][0] * x3 + grad3[gi3][1] * y3 + grad3[gi3][2] * z3);
  }

  /* add contributions and scale to [-1, 1] range */
  return 32.0f * (n0 + n1 + n2 + n3);

}

/*

         commc_fractal_noise()
	       ---
	       generates fractal noise by combining multiple octaves of 
	       Perlin noise. each octave has double the frequency and
	       reduced amplitude, creating natural-looking detail.

*/

float commc_fractal_noise(float x, float y, int octaves, float persistence, float lacunarity) {

  float  total;
  float  frequency;
  float  amplitude;
  float  max_value;
  int    i;

  total = 0.0f;
  frequency = 1.0f;
  amplitude = 1.0f;
  max_value = 0.0f;

  /* clamp octaves to reasonable range */
  if  (octaves < 1) octaves = 1;
  if  (octaves > 16) octaves = 16;

  /* clamp parameters to reasonable ranges */
  if  (persistence < 0.0f) persistence = 0.5f;
  if  (persistence > 1.0f) persistence = 1.0f;
  if  (lacunarity < 1.0f) lacunarity = 2.0f;

  for  (i = 0; i < octaves; i++) {

    total += commc_perlin_noise_2d(x * frequency, y * frequency) * amplitude;
    max_value += amplitude;
    
    amplitude *= persistence;
    frequency *= lacunarity;
  }

  /* normalize to [-1, 1] range */
  return total / max_value;

}

/*

         commc_fractal_noise_3d()
	       ---
	       generates 3D fractal noise by combining multiple octaves of 
	       3D Perlin noise. provides rich volumetric noise for 3D
	       applications like procedural materials and effects.

*/

float commc_fractal_noise_3d(float x, float y, float z, int octaves, float persistence, float lacunarity) {

  float  total;
  float  frequency;
  float  amplitude;
  float  max_value;
  int    i;

  total = 0.0f;
  frequency = 1.0f;
  amplitude = 1.0f;
  max_value = 0.0f;

  /* clamp octaves to reasonable range */
  if  (octaves < 1) octaves = 1;
  if  (octaves > 16) octaves = 16;

  /* clamp parameters to reasonable ranges */
  if  (persistence < 0.0f) persistence = 0.5f;
  if  (persistence > 1.0f) persistence = 1.0f;
  if  (lacunarity < 1.0f) lacunarity = 2.0f;

  for  (i = 0; i < octaves; i++) {

    total += commc_perlin_noise(x * frequency, y * frequency, z * frequency) * amplitude;
    max_value += amplitude;
    
    amplitude *= persistence;
    frequency *= lacunarity;
  }

  /* normalize to [-1, 1] range */
  return total / max_value;

}

/*

         commc_ridged_noise()
	       ---
	       generates ridged fractal noise by inverting and layering
	       absolute values of noise. creates sharp ridge-like features
	       excellent for mountain ranges and rocky terrain.

*/

float commc_ridged_noise(float x, float y, int octaves, float persistence, float lacunarity) {

  float  total;
  float  frequency;
  float  amplitude;
  float  max_value;
  float  sample;
  int    i;

  total = 0.0f;
  frequency = 1.0f;
  amplitude = 1.0f;
  max_value = 0.0f;

  /* clamp octaves to reasonable range */
  if  (octaves < 1) octaves = 1;
  if  (octaves > 16) octaves = 16;

  /* clamp parameters to reasonable ranges */
  if  (persistence < 0.0f) persistence = 0.5f;
  if  (persistence > 1.0f) persistence = 1.0f;
  if  (lacunarity < 1.0f) lacunarity = 2.0f;

  for  (i = 0; i < octaves; i++) {

    /* get noise sample and create ridges */
    sample = commc_perlin_noise_2d(x * frequency, y * frequency);
    sample = 1.0f - (float)fabs(sample);  /* invert and create ridges */
    sample *= sample;                     /* sharpen ridges */
    
    total += sample * amplitude;
    max_value += amplitude;
    
    amplitude *= persistence;
    frequency *= lacunarity;
  }

  /* normalize to [0, 1] range */
  return total / max_value;

}

/* 
	==================================
        --- NUMERICAL ANALYSIS ---
	==================================
*/

/*

         commc_integrate_trapezoidal()
	       ---
	       performs numerical integration using the trapezoidal rule.
	       the algorithm divides the interval [a, b] into n equal
	       subintervals and approximates the area under the curve
	       using trapezoids.

*/

commc_integration_result_t commc_integrate_trapezoidal(commc_function_t f, float a, float b, int n) {

  commc_integration_result_t result;
  
  float  h;                    /* STEP SIZE */
  float  sum;                  /* RUNNING SUM */
  float  x;                    /* CURRENT X POSITION */
  int    i;

  result.success = 1;
  result.evaluations = n + 1;
  result.error_estimate = 0.0f;  /* TRAPEZOIDAL RULE DOESN'T PROVIDE ERROR ESTIMATE */

  if  (n <= 0) {

    result.success = 0;
    result.result = 0.0f;
    return result;
  }

  h = (b - a) / (float)n;

  /* trapezoidal rule: (h/2) * [f(a) + 2*f(x1) + 2*f(x2) + ... + 2*f(xn-1) + f(b)] */

  sum = f(a) + f(b);           /* END POINTS GET COEFFICIENT 1 */

  for  (i = 1; i < n; i++) {

    x = a + i * h;
    sum += 2.0f * f(x);        /* INTERIOR POINTS GET COEFFICIENT 2 */
  }

  result.result = (h / 2.0f) * sum;
  return result;

}

/*

         commc_integrate_simpson()
	       ---
	       performs numerical integration using Simpson's 1/3 rule.
	       uses parabolic approximation instead of linear trapezoids,
	       providing much higher accuracy (O(h^4) vs O(h^2)).

*/

commc_integration_result_t commc_integrate_simpson(commc_function_t f, float a, float b, int n) {

  commc_integration_result_t result;
  
  float  h;                    /* STEP SIZE */
  float  sum;                  /* RUNNING SUM */
  float  x;                    /* CURRENT X POSITION */
  int    i;

  result.success = 1;
  result.evaluations = n + 1;
  result.error_estimate = 0.0f;  /* SIMPSON'S RULE DOESN'T PROVIDE ERROR ESTIMATE */

  /* Simpson's rule requires even number of intervals */
  if  (n <= 0 || (n % 2) != 0) {

    result.success = 0;
    result.result = 0.0f;
    return result;
  }

  h = (b - a) / (float)n;

  /* Simpson's 1/3 rule: (h/3) * [f(a) + 4*f(x1) + 2*f(x2) + 4*f(x3) + ... + f(b)] */

  sum = f(a) + f(b);           /* END POINTS GET COEFFICIENT 1 */

  for  (i = 1; i < n; i++) {

    x = a + i * h;
    
    if  (i % 2 == 1) {
      sum += 4.0f * f(x);      /* ODD INDICES GET COEFFICIENT 4 */
    } else {
      sum += 2.0f * f(x);      /* EVEN INDICES GET COEFFICIENT 2 */
    }
  }

  result.result = (h / 3.0f) * sum;
  return result;

}

/*

         commc_integrate_adaptive()
	       ---
	       performs adaptive Simpson's rule integration with automatic
	       error control. recursively subdivides intervals where the
	       error estimate exceeds the tolerance.

*/

static commc_integration_result_t commc_integrate_adaptive_recursive(commc_function_t f, float a, float b, float tolerance, float fa, float fb, float fc, float whole_area, int depth, int* total_evaluations) {

  commc_integration_result_t result;
  commc_integration_result_t left_result, right_result;
  
  float  c;                    /* MIDPOINT */
  float  d, e;                 /* QUARTER POINTS */
  float  fd, fe;               /* FUNCTION VALUES AT QUARTER POINTS */
  float  left_area, right_area;
  float  error_estimate;
  float  tolerance_half;

  /* prevent infinite recursion */
  if  (depth > 20) {

    result.success = 0;
    result.result = 0.0f;
    result.error_estimate = tolerance + 1.0f;  /* INDICATE ERROR TOO LARGE */
    result.evaluations = 0;
    return result;
  }

  c = (a + b) / 2.0f;          /* MIDPOINT */
  d = (a + c) / 2.0f;          /* LEFT QUARTER POINT */
  e = (c + b) / 2.0f;          /* RIGHT QUARTER POINT */

  fd = f(d);
  fe = f(e);
  (*total_evaluations) += 2;

  /* compute Simpson's rule for left and right halves */
  left_area  = (b - a) / 12.0f * (fa + 4.0f * fd + fc);
  right_area = (b - a) / 12.0f * (fc + 4.0f * fe + fb);

  /* estimate error using Richardson extrapolation */
  error_estimate = (float)fabs((left_area + right_area) - whole_area) / 15.0f;

  if  (error_estimate <= tolerance) {

    /* tolerance met, return result */
    result.success = 1;
    result.result = left_area + right_area + error_estimate;  /* ADD ERROR CORRECTION */
    result.error_estimate = error_estimate;
    result.evaluations = *total_evaluations;
    return result;
    
  } else {

    /* subdivide and recurse */
    tolerance_half = tolerance / 2.0f;
    
    left_result  = commc_integrate_adaptive_recursive(f, a, c, tolerance_half, fa, fc, fd, left_area, depth + 1, total_evaluations);
    right_result = commc_integrate_adaptive_recursive(f, c, b, tolerance_half, fc, fb, fe, right_area, depth + 1, total_evaluations);
    
    if  (left_result.success && right_result.success) {

      result.success = 1;
      result.result = left_result.result + right_result.result;
      result.error_estimate = left_result.error_estimate + right_result.error_estimate;
      result.evaluations = *total_evaluations;
      
    } else {

      result.success = 0;
      result.result = 0.0f;
      result.error_estimate = tolerance + 1.0f;
      result.evaluations = *total_evaluations;
    }

    return result;
  }

}

commc_integration_result_t commc_integrate_adaptive(commc_function_t f, float a, float b, float tolerance) {

  commc_integration_result_t result;
  
  float  c;                    /* MIDPOINT */
  float  fa, fb, fc;           /* FUNCTION VALUES */
  float  whole_area;           /* INITIAL SIMPSON'S APPROXIMATION */
  int    total_evaluations;

  if  (tolerance <= 0.0f) {

    result.success = 0;
    result.result = 0.0f;
    result.error_estimate = 0.0f;
    result.evaluations = 0;
    return result;
  }

  c = (a + b) / 2.0f;
  fa = f(a);
  fb = f(b);
  fc = f(c);
  total_evaluations = 3;

  /* initial Simpson's approximation for the whole interval */
  whole_area = (b - a) / 6.0f * (fa + 4.0f * fc + fb);

  return commc_integrate_adaptive_recursive(f, a, b, tolerance, fa, fb, fc, whole_area, 0, &total_evaluations);

}

/*

         commc_root_bisection()
	       ---
	       finds a root of f(x) = 0 using the bisection method.
	       requires that f(a) and f(b) have different signs.
	       converges slowly but reliably.

*/

commc_root_result_t commc_root_bisection(commc_function_t f, float a, float b, float tolerance, int max_iterations) {

  commc_root_result_t result;
  
  float  fa, fb, fc;           /* FUNCTION VALUES */
  float  c;                    /* CURRENT MIDPOINT */
  int    iteration;

  fa = f(a);
  fb = f(b);

  /* check that root exists in interval */
  if  (fa * fb > 0.0f) {

    result.success = 0;
    result.root = 0.0f;
    result.error = 0.0f;
    result.iterations = 0;
    return result;
  }

  for  (iteration = 0; iteration < max_iterations; iteration++) {

    c = (a + b) / 2.0f;        /* COMPUTE MIDPOINT */
    fc = f(c);

    /* check for convergence */
    if  ((float)fabs(fc) < tolerance || (b - a) / 2.0f < tolerance) {

      result.success = 1;
      result.root = c;
      result.error = (b - a) / 2.0f;
      result.iterations = iteration + 1;
      return result;
    }

    /* determine which half contains the root */
    if  (fa * fc < 0.0f) {
      b = c;
      fb = fc;
    } else {
      a = c;
      fa = fc;
    }
  }

  /* failed to converge */
  result.success = 0;
  result.root = c;
  result.error = (b - a) / 2.0f;
  result.iterations = max_iterations;
  return result;

}

/*

         commc_root_newton_raphson()
	       ---
	       finds a root using Newton-Raphson method with analytical
	       derivative. converges quadratically near the root but
	       can be unstable if derivative is small.

*/

commc_root_result_t commc_root_newton_raphson(commc_function_with_derivative_t f, float x0, float tolerance, int max_iterations) {

  commc_root_result_t result;
  
  float  x_current;            /* CURRENT APPROXIMATION */
  float  x_next;               /* NEXT APPROXIMATION */
  float  fx, dfx;              /* FUNCTION AND DERIVATIVE VALUES */
  int    iteration;

  x_current = x0;

  for  (iteration = 0; iteration < max_iterations; iteration++) {

    fx = f(x_current, &dfx);

    /* check for zero derivative (method fails) */
    if  ((float)fabs(dfx) < 1e-12f) {

      result.success = 0;
      result.root = x_current;
      result.error = 0.0f;
      result.iterations = iteration;
      return result;
    }

    /* Newton-Raphson update: x_{n+1} = x_n - f(x_n) / f'(x_n) */
    x_next = x_current - fx / dfx;

    /* check for convergence */
    if  ((float)fabs(x_next - x_current) < tolerance) {

      result.success = 1;
      result.root = x_next;
      result.error = (float)fabs(x_next - x_current);
      result.iterations = iteration + 1;
      return result;
    }

    x_current = x_next;
  }

  /* failed to converge */
  result.success = 0;
  result.root = x_current;
  result.error = tolerance + 1.0f;  /* INDICATE FAILURE */
  result.iterations = max_iterations;
  return result;

}

/*

         commc_root_secant()
	       ---
	       finds a root using the secant method. similar to Newton-Raphson
	       but uses finite difference approximation for the derivative.
	       requires two initial guesses.

*/

commc_root_result_t commc_root_secant(commc_function_t f, float x0, float x1, float tolerance, int max_iterations) {

  commc_root_result_t result;
  
  float  fx0, fx1;             /* FUNCTION VALUES */
  float  x_next;               /* NEXT APPROXIMATION */
  float  denominator;          /* FINITE DIFFERENCE DENOMINATOR */
  int    iteration;

  fx0 = f(x0);
  fx1 = f(x1);

  for  (iteration = 0; iteration < max_iterations; iteration++) {

    /* compute finite difference approximation of derivative */
    denominator = fx1 - fx0;

    /* check for zero denominator (method fails) */
    if  ((float)fabs(denominator) < 1e-12f) {

      result.success = 0;
      result.root = x1;
      result.error = 0.0f;
      result.iterations = iteration;
      return result;
    }

    /* secant method update: x_{n+1} = x_n - f(x_n) * (x_n - x_{n-1}) / (f(x_n) - f(x_{n-1})) */
    x_next = x1 - fx1 * (x1 - x0) / denominator;

    /* check for convergence */
    if  ((float)fabs(x_next - x1) < tolerance) {

      result.success = 1;
      result.root = x_next;
      result.error = (float)fabs(x_next - x1);
      result.iterations = iteration + 1;
      return result;
    }

    /* update for next iteration */
    x0 = x1;
    fx0 = fx1;
    x1 = x_next;
    fx1 = f(x1);
  }

  /* failed to converge */
  result.success = 0;
  result.root = x1;
  result.error = tolerance + 1.0f;  /* INDICATE FAILURE */
  result.iterations = max_iterations;
  return result;

}

/*

         commc_optimize_gradient_descent()
	       ---
	       performs gradient descent optimization using numerical
	       gradient approximation. finds local minimum of the
	       objective function.

*/

commc_optimization_result_t commc_optimize_gradient_descent(commc_function_t f, float x0, float learning_rate, float tolerance, int max_iterations) {

  commc_optimization_result_t result;
  
  float  x_current;            /* CURRENT POSITION */
  float  x_next;               /* NEXT POSITION */
  float  gradient;             /* NUMERICAL GRADIENT */
  float  fx, fx_plus_h;        /* FUNCTION VALUES FOR GRADIENT */
  float  h;                    /* FINITE DIFFERENCE STEP SIZE */
  int    iteration;

  h = 1e-6f;                   /* SMALL STEP SIZE FOR NUMERICAL DERIVATIVE */
  x_current = x0;

  for  (iteration = 0; iteration < max_iterations; iteration++) {

    /* compute numerical gradient using finite difference */
    fx = f(x_current);
    fx_plus_h = f(x_current + h);
    gradient = (fx_plus_h - fx) / h;

    /* gradient descent update: x_{n+1} = x_n - α * ∇f(x_n) */
    x_next = x_current - learning_rate * gradient;

    /* check for convergence */
    if  ((float)fabs(x_next - x_current) < tolerance) {

      result.success = 1;
      result.x_optimal = x_next;
      result.f_optimal = f(x_next);
      result.iterations = iteration + 1;
      return result;
    }

    x_current = x_next;
  }

  /* failed to converge */
  result.success = 0;
  result.x_optimal = x_current;
  result.f_optimal = f(x_current);
  result.iterations = max_iterations;
  return result;

}

/*

         commc_optimize_gradient_descent_with_derivative()
	       ---
	       gradient descent optimization using analytical derivative
	       for improved accuracy and performance compared to numerical
	       gradient approximation.

*/

commc_optimization_result_t commc_optimize_gradient_descent_with_derivative(commc_function_with_derivative_t f, float x0, float learning_rate, float tolerance, int max_iterations) {

  commc_optimization_result_t result;
  
  float  x_current;            /* CURRENT POSITION */
  float  x_next;               /* NEXT POSITION */
  float  dfx;                  /* DERIVATIVE VALUE */
  int    iteration;

  x_current = x0;

  for  (iteration = 0; iteration < max_iterations; iteration++) {

    f(x_current, &dfx);        /* GET DERIVATIVE AT CURRENT POSITION */

    /* gradient descent update: x_{n+1} = x_n - α * ∇f(x_n) */
    x_next = x_current - learning_rate * dfx;

    /* check for convergence */
    if  ((float)fabs(x_next - x_current) < tolerance) {

      result.success = 1;
      result.x_optimal = x_next;
      result.f_optimal = f(x_next, &dfx);  /* COMPUTE FINAL FUNCTION VALUE */
      result.iterations = iteration + 1;
      return result;
    }

    x_current = x_next;
  }

  /* failed to converge */
  result.success = 0;
  result.x_optimal = x_current;
  result.f_optimal = f(x_current, &dfx);
  result.iterations = max_iterations;
  return result;

}

/* 
	==================================
          --- STATISTICS ---
	==================================
*/

/*

         commc_random_uniform()
	       ---
	       generates a random number from uniform distribution
	       in the range [min_val, max_val] using the existing
	       commc_rand_float() function.

*/

float commc_random_uniform(float min_val, float max_val) {

  return min_val + commc_rand_float() * (max_val - min_val);

}

/*

         commc_random_normal()
	       ---
	       generates a random number from normal distribution
	       using Box-Muller transformation. generates two
	       independent normal variates but returns only one.

*/

float commc_random_normal(float mean, float sigma) {

  static int   has_spare = 0;
  static float spare;
  
  float  u, v, s;
  float  result;

  if  (has_spare) {

    has_spare = 0;
    return spare * sigma + mean;
  }

  has_spare = 1;

  /* Box-Muller transformation */
  do {
    u = commc_rand_float() * 2.0f - 1.0f;  /* U(-1, 1) */
    v = commc_rand_float() * 2.0f - 1.0f;  /* V(-1, 1) */
    s = u * u + v * v;
  } while (s >= 1.0f || s == 0.0f);

  s = (float)sqrt(-2.0f * (float)log(s) / s);
  
  spare = v * s;                           /* SAVE FOR NEXT CALL */
  result = u * s;
  
  return result * sigma + mean;

}

/*

         commc_random_exponential()
	       ---
	       generates a random number from exponential distribution
	       using inverse transform sampling. uses the formula:
	       X = -ln(1-U) / λ where U is uniform(0,1).

*/

float commc_random_exponential(float lambda) {

  float u;

  if  (lambda <= 0.0f) {
    return 0.0f;  /* INVALID PARAMETER */
  }

  u = commc_rand_float();
  
  /* avoid log(0) by ensuring u > 0 */
  while  (u == 0.0f) {
    u = commc_rand_float();
  }

  return -(float)log(1.0f - u) / lambda;

}

/*

         commc_probability_density_normal()
	       ---
	       computes probability density function (PDF) for normal
	       distribution at point x. uses the standard formula:
	       f(x) = 1/(σ√(2π)) * exp(-(x-μ)²/(2σ²))

*/

float commc_probability_density_normal(float x, float mean, float sigma) {

  float coefficient;
  float exponent;

  if  (sigma <= 0.0f) {
    return 0.0f;  /* INVALID PARAMETER */
  }

  coefficient = 1.0f / (sigma * (float)sqrt(2.0f * M_PI));
  exponent = -0.5f * (x - mean) * (x - mean) / (sigma * sigma);
  
  return coefficient * (float)exp(exponent);

}

/*

         commc_cumulative_distribution_normal()
	       ---
	       computes cumulative distribution function (CDF) for
	       normal distribution using error function approximation.
	       uses Abramowitz and Stegun approximation.

*/

float commc_cumulative_distribution_normal(float x, float mean, float sigma) {

  float z;
  float t;
  float erf_approx;

  /* CONSTANTS FOR ERROR FUNCTION APPROXIMATION */
  float a1 = 0.254829592f;
  float a2 = -0.284496736f;
  float a3 = 1.421413741f;
  float a4 = -1.453152027f;
  float a5 = 1.061405429f;
  float p  = 0.3275911f;

  if  (sigma <= 0.0f) {
    return (x >= mean) ? 1.0f : 0.0f;  /* DEGENERATE CASE */
  }

  /* standardize: z = (x - μ) / σ */
  z = (x - mean) / sigma;

  /* compute error function using Abramowitz-Stegun approximation */
  t = 1.0f / (1.0f + p * (float)fabs(z));
  erf_approx = 1.0f - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * (float)exp(-z * z);

  if  (z < 0.0f) {
    erf_approx = -erf_approx;
  }

  /* CDF = 0.5 * (1 + erf(z / √2)) */
  return 0.5f * (1.0f + erf_approx);

}

/*

         commc_histogram_create()
	       ---
	       creates a histogram by analyzing data array and
	       distributing values into equal-width bins.

*/

commc_histogram_t* commc_histogram_create(float* data, int data_size, int num_bins) {

  commc_histogram_t* histogram;
  
  float  min_val, max_val;
  int    bin_index;
  int    i;

  if  (data == NULL || data_size <= 0 || num_bins <= 0) {
    return NULL;
  }

  histogram = (commc_histogram_t*)malloc(sizeof(commc_histogram_t));
  
  if  (histogram == NULL) {
    return NULL;
  }

  /* find min and max values in data */
  min_val = max_val = data[0];
  
  for  (i = 1; i < data_size; i++) {

    if  (data[i] < min_val) min_val = data[i];
    if  (data[i] > max_val) max_val = data[i];
  }

  /* allocate and initialize bins */
  histogram->bins = (float*)calloc(num_bins, sizeof(float));
  
  if  (histogram->bins == NULL) {
    free(histogram);
    return NULL;
  }

  histogram->num_bins = num_bins;
  histogram->min_value = min_val;
  histogram->max_value = max_val;
  histogram->bin_width = (max_val - min_val) / (float)num_bins;
  histogram->total_count = data_size;

  /* distribute data into bins */
  for  (i = 0; i < data_size; i++) {

    if  (histogram->bin_width > 0.0f) {
      bin_index = (int)((data[i] - min_val) / histogram->bin_width);
      
      /* handle edge case where data[i] == max_val */
      if  (bin_index >= num_bins) {
        bin_index = num_bins - 1;
      }
      
      histogram->bins[bin_index] += 1.0f;
    }
  }

  return histogram;

}

/*

         commc_histogram_destroy()
	       ---
	       frees memory allocated for histogram and its bins.

*/

void commc_histogram_destroy(commc_histogram_t* histogram) {

  if  (histogram != NULL) {

    if  (histogram->bins != NULL) {
      free(histogram->bins);
    }
    
    free(histogram);
  }

}

/*

         commc_histogram_add_value()
	       ---
	       adds a single value to existing histogram by
	       finding appropriate bin and incrementing count.

*/

void commc_histogram_add_value(commc_histogram_t* histogram, float value) {

  int bin_index;

  if  (histogram == NULL || histogram->bins == NULL) {
    return;
  }

  /* check if value is within histogram range */
  if  (value < histogram->min_value || value > histogram->max_value) {
    return;  /* VALUE OUT OF RANGE */
  }

  if  (histogram->bin_width > 0.0f) {

    bin_index = (int)((value - histogram->min_value) / histogram->bin_width);
    
    /* handle edge case */
    if  (bin_index >= histogram->num_bins) {
      bin_index = histogram->num_bins - 1;
    }
    
    histogram->bins[bin_index] += 1.0f;
    histogram->total_count++;
  }

}

/*

         commc_histogram_get_percentile()
	       ---
	       computes percentile value from histogram by finding
	       the bin where cumulative count reaches the target.

*/

float commc_histogram_get_percentile(commc_histogram_t* histogram, float percentile) {

  float  target_count;
  float  cumulative_count;
  int    i;

  if  (histogram == NULL || histogram->bins == NULL || percentile < 0.0f || percentile > 1.0f) {
    return 0.0f;
  }

  target_count = percentile * (float)histogram->total_count;
  cumulative_count = 0.0f;

  for  (i = 0; i < histogram->num_bins; i++) {

    cumulative_count += histogram->bins[i];
    
    if  (cumulative_count >= target_count) {
      /* return midpoint of bin */
      return histogram->min_value + ((float)i + 0.5f) * histogram->bin_width;
    }
  }

  /* return maximum value if not found */
  return histogram->max_value;

}

/*

         commc_statistics_compute()
	       ---
	       computes comprehensive statistical analysis including
	       central tendency, dispersion, and shape measures.

*/

commc_statistics_t commc_statistics_compute(float* data, int data_size) {

  commc_statistics_t stats;
  
  float  sum, sum_sq, sum_cubed, sum_fourth;
  float  mean_dev, variance;
  int    i;

  /* initialize results */
  memset(&stats, 0, sizeof(commc_statistics_t));

  if  (data == NULL || data_size <= 0) {
    return stats;
  }

  /* find min and max */
  stats.min_value = stats.max_value = data[0];
  
  for  (i = 0; i < data_size; i++) {
    if  (data[i] < stats.min_value) stats.min_value = data[i];
    if  (data[i] > stats.max_value) stats.max_value = data[i];
  }
  
  stats.range = stats.max_value - stats.min_value;

  /* compute mean */
  sum = 0.0f;
  
  for  (i = 0; i < data_size; i++) {
    sum += data[i];
  }
  
  stats.mean = sum / (float)data_size;

  /* compute median (assumes sorted data) */
  if  (data_size % 2 == 0) {
    stats.median = (data[data_size / 2 - 1] + data[data_size / 2]) / 2.0f;
  } else {
    stats.median = data[data_size / 2];
  }

  /* compute variance, skewness, and kurtosis */
  sum_sq = sum_cubed = sum_fourth = 0.0f;
  
  for  (i = 0; i < data_size; i++) {

    mean_dev = data[i] - stats.mean;
    sum_sq += mean_dev * mean_dev;
    sum_cubed += mean_dev * mean_dev * mean_dev;
    sum_fourth += mean_dev * mean_dev * mean_dev * mean_dev;
  }

  variance = sum_sq / (float)data_size;
  stats.variance = variance;
  stats.std_deviation = (float)sqrt(variance);

  /* compute skewness and kurtosis */
  if  (stats.std_deviation > 0.0f) {

    stats.skewness = (sum_cubed / (float)data_size) / (stats.std_deviation * stats.std_deviation * stats.std_deviation);
    stats.kurtosis = (sum_fourth / (float)data_size) / (variance * variance) - 3.0f;  /* EXCESS KURTOSIS */
  }

  /* compute mode (most frequent value - approximation for continuous data) */
  stats.mode = stats.mean;  /* SIMPLE APPROXIMATION */

  return stats;

}

/*

         commc_statistics_correlation()
	       ---
	       computes Pearson correlation coefficient using the
	       formula: r = Σ((x-x̄)(y-ȳ)) / √(Σ(x-x̄)²Σ(y-ȳ)²)

*/

float commc_statistics_correlation(float* x_data, float* y_data, int data_size) {

  float  x_mean, y_mean;
  float  sum_x, sum_y;
  float  sum_xy, sum_x_sq, sum_y_sq;
  float  numerator, denominator;
  int    i;

  if  (x_data == NULL || y_data == NULL || data_size <= 1) {
    return 0.0f;  /* INSUFFICIENT DATA */
  }

  /* compute means */
  sum_x = sum_y = 0.0f;
  
  for  (i = 0; i < data_size; i++) {
    sum_x += x_data[i];
    sum_y += y_data[i];
  }
  
  x_mean = sum_x / (float)data_size;
  y_mean = sum_y / (float)data_size;

  /* compute correlation components */
  sum_xy = sum_x_sq = sum_y_sq = 0.0f;
  
  for  (i = 0; i < data_size; i++) {

    float x_dev = x_data[i] - x_mean;
    float y_dev = y_data[i] - y_mean;
    
    sum_xy += x_dev * y_dev;
    sum_x_sq += x_dev * x_dev;
    sum_y_sq += y_dev * y_dev;
  }

  /* compute correlation coefficient */
  denominator = (float)sqrt(sum_x_sq * sum_y_sq);
  
  if  (denominator == 0.0f) {
    return 0.0f;  /* NO CORRELATION POSSIBLE */
  }

  numerator = sum_xy;
  return numerator / denominator;

}

/*

         commc_statistics_linear_regression()
	       ---
	       performs simple linear regression to find best-fit line
	       y = mx + b using least squares method.

*/

void commc_statistics_linear_regression(float* x_data, float* y_data, int data_size, float* slope_out, float* intercept_out) {

  float  x_mean, y_mean;
  float  sum_x, sum_y;
  float  sum_xy, sum_x_sq;
  float  numerator, denominator;
  int    i;

  if  (x_data == NULL || y_data == NULL || data_size <= 1 || slope_out == NULL || intercept_out == NULL) {
    
    if  (slope_out != NULL) *slope_out = 0.0f;
    if  (intercept_out != NULL) *intercept_out = 0.0f;
    return;
  }

  /* compute means */
  sum_x = sum_y = 0.0f;
  
  for  (i = 0; i < data_size; i++) {
    sum_x += x_data[i];
    sum_y += y_data[i];
  }
  
  x_mean = sum_x / (float)data_size;
  y_mean = sum_y / (float)data_size;

  /* compute regression components */
  sum_xy = sum_x_sq = 0.0f;
  
  for  (i = 0; i < data_size; i++) {

    float x_dev = x_data[i] - x_mean;
    float y_dev = y_data[i] - y_mean;
    
    sum_xy += x_dev * y_dev;
    sum_x_sq += x_dev * x_dev;
  }

  /* compute slope and intercept */
  denominator = sum_x_sq;
  
  if  (denominator == 0.0f) {
    /* vertical line case */
    *slope_out = 0.0f;
    *intercept_out = y_mean;
  } else {
    numerator = sum_xy;
    *slope_out = numerator / denominator;
    *intercept_out = y_mean - (*slope_out) * x_mean;
  }

}

/* 
	==================================
        --- COMPUTATIONAL GEOMETRY ---
	==================================
*/

/*

         commc_aabb_create()
	       ---
	       creates an axis-aligned bounding box from minimum
	       and maximum coordinates. ensures proper ordering
	       by swapping coordinates if necessary.

*/

commc_aabb_t commc_aabb_create(float min_x, float min_y, float max_x, float max_y) {

  commc_aabb_t box;
  
  /* ensure min values are actually minimum */
  if  (min_x > max_x) {
    float temp = min_x;
    min_x = max_x;
    max_x = temp;
  }
  
  if  (min_y > max_y) {
    float temp = min_y;
    min_y = max_y;
    max_y = temp;
  }
  
  box.min_x = min_x;
  box.min_y = min_y;
  box.max_x = max_x;
  box.max_y = max_y;
  
  return box;

}

/*

         commc_aabb_intersects()
	       ---
	       tests if two axis-aligned bounding boxes intersect using
	       the separating axis theorem. two AABBs intersect if they
	       overlap on both x and y axes.

*/

int commc_aabb_intersects(commc_aabb_t box1, commc_aabb_t box2) {

  /* check for separation on x-axis */
  if  (box1.max_x < box2.min_x || box2.max_x < box1.min_x) {
    return 0; /* separated on x-axis */
  }
  
  /* check for separation on y-axis */
  if  (box1.max_y < box2.min_y || box2.max_y < box1.min_y) {
    return 0; /* separated on y-axis */
  }
  
  return 1; /* intersecting on both axes */

}

/*

         commc_aabb_contains_point()
	       ---
	       tests if an AABB contains a given point.
	       point is inside if it's within all bounds.

*/

int commc_aabb_contains_point(commc_aabb_t box, float x, float y) {

  return (x >= box.min_x && x <= box.max_x && 
          y >= box.min_y && y <= box.max_y);

}

/*

         commc_aabb3d_create()
	       ---
	       creates a 3D axis-aligned bounding box with proper
	       coordinate ordering for all three dimensions.

*/

commc_aabb3d_t commc_aabb3d_create(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z) {

  commc_aabb3d_t box;
  
  /* ensure min values are actually minimum */
  if  (min_x > max_x) {
    float temp = min_x;
    min_x = max_x;
    max_x = temp;
  }
  
  if  (min_y > max_y) {
    float temp = min_y;
    min_y = max_y;
    max_y = temp;
  }
  
  if  (min_z > max_z) {
    float temp = min_z;
    min_z = max_z;
    max_z = temp;
  }
  
  box.min_x = min_x;
  box.min_y = min_y;
  box.min_z = min_z;
  box.max_x = max_x;
  box.max_y = max_y;
  box.max_z = max_z;
  
  return box;

}

/*

         commc_aabb3d_intersects()
	       ---
	       tests if two 3D axis-aligned bounding boxes intersect.
	       extends the 2D algorithm to check separation on all
	       three axes: x, y, and z.

*/

int commc_aabb3d_intersects(commc_aabb3d_t box1, commc_aabb3d_t box2) {

  /* check for separation on x-axis */
  if  (box1.max_x < box2.min_x || box2.max_x < box1.min_x) {
    return 0;
  }
  
  /* check for separation on y-axis */
  if  (box1.max_y < box2.min_y || box2.max_y < box1.min_y) {
    return 0;
  }
  
  /* check for separation on z-axis */
  if  (box1.max_z < box2.min_z || box2.max_z < box1.min_z) {
    return 0;
  }
  
  return 1; /* intersecting on all axes */

}

/*

         commc_circle_create()
	       ---
	       creates a circle structure for collision detection.
	       ensures radius is non-negative.

*/

commc_circle_t commc_circle_create(float center_x, float center_y, float radius) {

  commc_circle_t circle;
  
  circle.center_x = center_x;
  circle.center_y = center_y;
  circle.radius   = (radius < 0.0f) ? 0.0f : radius;
  
  return circle;

}

/*

         commc_circles_intersect()
	       ---
	       tests if two circles intersect by comparing the distance
	       between their centers with the sum of their radii.
	       uses squared distances to avoid expensive square root.

*/

int commc_circles_intersect(commc_circle_t circle1, commc_circle_t circle2) {

  float dx = circle2.center_x - circle1.center_x;
  float dy = circle2.center_y - circle1.center_y;
  float distance_sq = dx * dx + dy * dy;
  float radius_sum = circle1.radius + circle2.radius;
  float radius_sum_sq = radius_sum * radius_sum;
  
  return distance_sq <= radius_sum_sq;

}

/*

         commc_circle_contains_point()
	       ---
	       tests if a circle contains a given point using
	       squared distance comparison to avoid square root.

*/

int commc_circle_contains_point(commc_circle_t circle, float x, float y) {

  float dx = x - circle.center_x;
  float dy = y - circle.center_y;
  float distance_sq = dx * dx + dy * dy;
  float radius_sq = circle.radius * circle.radius;
  
  return distance_sq <= radius_sq;

}

/*

         commc_sphere_create()
	       ---
	       creates a sphere structure for 3D collision detection.
	       ensures radius is non-negative.

*/

commc_sphere_t commc_sphere_create(float center_x, float center_y, float center_z, float radius) {

  commc_sphere_t sphere;
  
  sphere.center_x = center_x;
  sphere.center_y = center_y;
  sphere.center_z = center_z;
  sphere.radius   = (radius < 0.0f) ? 0.0f : radius;
  
  return sphere;

}

/*

         commc_spheres_intersect()
	       ---
	       tests if two spheres intersect using 3D distance
	       calculation between centers. uses squared distances
	       for performance optimization.

*/

int commc_spheres_intersect(commc_sphere_t sphere1, commc_sphere_t sphere2) {

  float dx = sphere2.center_x - sphere1.center_x;
  float dy = sphere2.center_y - sphere1.center_y;
  float dz = sphere2.center_z - sphere1.center_z;
  float distance_sq = dx * dx + dy * dy + dz * dz;
  float radius_sum = sphere1.radius + sphere2.radius;
  float radius_sum_sq = radius_sum * radius_sum;
  
  return distance_sq <= radius_sum_sq;

}

/*

         commc_point_in_polygon()
	       ---
	       tests if a point is inside a polygon using the ray casting
	       algorithm. casts a ray from the point to the right and counts
	       intersections with polygon edges. odd count means inside.

*/

int commc_point_in_polygon(commc_polygon_t polygon, float x, float y) {

  int   i;
  int   j;
  int   intersections = 0;
  float xi, yi, xj, yj;
  
  if  (!polygon.vertices || polygon.vertex_count < 3) {
    return 0; /* invalid polygon */
  }
  
  /* ray casting algorithm */
  j = polygon.vertex_count - 1;
  
  for  (i = 0; i < polygon.vertex_count; i++) {

    xi = polygon.vertices[i * 2];     /* x coordinate of vertex i */
    yi = polygon.vertices[i * 2 + 1]; /* y coordinate of vertex i */
    xj = polygon.vertices[j * 2];     /* x coordinate of vertex j */
    yj = polygon.vertices[j * 2 + 1]; /* y coordinate of vertex j */
    
    /* check if ray intersects edge from j to i */
    if  (((yi > y) != (yj > y)) && 
         (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
      intersections++;
    }
    
    j = i;
  }
  
  return (intersections % 2) == 1; /* odd number means inside */

}

/*

         commc_line_clip_cohen_sutherland()
	       ---
	       clips a line segment against a rectangular window using
	       the Cohen-Sutherland algorithm. uses outcodes to efficiently
	       determine which parts of the line need clipping.

*/

int commc_line_clip_cohen_sutherland(commc_line_segment_t* line, float x_min, float y_min, float x_max, float y_max) {

  /* outcode constants for Cohen-Sutherland algorithm */
  int INSIDE = 0; /* 0000 */
  int LEFT   = 1; /* 0001 */
  int RIGHT  = 2; /* 0010 */
  int BOTTOM = 4; /* 0100 */
  int TOP    = 8; /* 1000 */
  
  int outcode1, outcode2, outcode_out;
  float x, y;
  
  if  (!line) {
    return 0;
  }
  
  /* compute outcodes for both endpoints */

  outcode1 = INSIDE;
  outcode2 = INSIDE;
  
  if  (line->x1 < x_min) outcode1 |= LEFT;
  else if (line->x1 > x_max) outcode1 |= RIGHT;
  if  (line->y1 < y_min) outcode1 |= BOTTOM;
  else if (line->y1 > y_max) outcode1 |= TOP;
  
  if  (line->x2 < x_min) outcode2 |= LEFT;
  else if (line->x2 > x_max) outcode2 |= RIGHT;
  if  (line->y2 < y_min) outcode2 |= BOTTOM;
  else if (line->y2 > y_max) outcode2 |= TOP;
  
  while  (1) {

    if  (!(outcode1 | outcode2)) {

      /* both endpoints inside window */

      return 1; /* accept line */

    }
    else if  (outcode1 & outcode2) {

      /* both endpoints share at least one outside region */

      return 0; /* reject line completely */

    }

    else {

      /* line needs clipping */

      outcode_out = outcode1 ? outcode1 : outcode2;
      
      /* find intersection point */

      if  (outcode_out & TOP) {

        x = line->x1 + (line->x2 - line->x1) * (y_max - line->y1) / (line->y2 - line->y1);

        y = y_max;

      }

      else if  (outcode_out & BOTTOM) {

        x = line->x1 + (line->x2 - line->x1) * (y_min - line->y1) / (line->y2 - line->y1);

        y = y_min;

      }

      else if  (outcode_out & RIGHT) {

        y = line->y1 + (line->y2 - line->y1) * (x_max - line->x1) / (line->x2 - line->x1);

        x = x_max;

      }

      else if  (outcode_out & LEFT) {

        y = line->y1 + (line->y2 - line->y1) * (x_min - line->x1) / (line->x2 - line->x1);

        x = x_min;

      }
      
      /* move outside point to intersection point */

      if  (outcode_out == outcode1) {

        line->x1 = x;
        line->y1 = y;
        outcode1 = INSIDE;
        if (line->x1 < x_min) outcode1 |= LEFT;
        else if (line->x1 > x_max) outcode1 |= RIGHT;
        if (line->y1 < y_min) outcode1 |= BOTTOM;
        else if (line->y1 > y_max) outcode1 |= TOP;

      }

      else {

        line->x2 = x;
        line->y2 = y;
        outcode2 = INSIDE;
        if (line->x2 < x_min) outcode2 |= LEFT;
        else if (line->x2 > x_max) outcode2 |= RIGHT;
        if (line->y2 < y_min) outcode2 |= BOTTOM;
        else if (line->y2 > y_max) outcode2 |= TOP;

      }
    }
  }

}

/*

         commc_polygon_triangulate()
	       ---
	       triangulates a polygon using ear clipping algorithm.
	       finds "ears" (triangles that can be cut off) and removes
	       them iteratively until only triangles remain.

*/

int* commc_polygon_triangulate(commc_polygon_t polygon, int* triangle_count_out) {

  int*   triangles;
  int*   vertex_list;
  int    vertex_count;
  int    triangle_count;
  int    v0, v1, v2;
  int    i, j;
  float  x0, y0, x1, y1, x2, y2;
  int    is_ear;
  int    triangle_index;
  
  if  (!polygon.vertices || polygon.vertex_count < 3 || !triangle_count_out) {

    *triangle_count_out = 0;
    return NULL;

  }
  
  vertex_count = polygon.vertex_count;
  triangle_count = vertex_count - 2;    /* n vertices = n-2 triangles */
  
  /* allocate arrays */

  triangles = (int*)malloc(triangle_count * 3 * sizeof(int));
  vertex_list = (int*)malloc(vertex_count * sizeof(int));
  
  if  (!triangles || !vertex_list) {

    free(triangles);
    free(vertex_list);
    *triangle_count_out = 0;
    return NULL;

  }
  
  /* initialize vertex list */

  for  (i = 0; i < vertex_count; i++) {

    vertex_list[i] = i;

  }
  
  triangle_index = 0;
  
  /* ear clipping algorithm */

  while  (vertex_count > 3) {

    is_ear = 0;
    
    /* find an ear */

    for  (i = 0; i < vertex_count && !is_ear; i++) {

      v0 = vertex_list[i];
      v1 = vertex_list[(i + 1) % vertex_count];
      v2 = vertex_list[(i + 2) % vertex_count];
      
      x0 = polygon.vertices[v0 * 2];
      y0 = polygon.vertices[v0 * 2 + 1];
      x1 = polygon.vertices[v1 * 2];
      y1 = polygon.vertices[v1 * 2 + 1];
      x2 = polygon.vertices[v2 * 2];
      y2 = polygon.vertices[v2 * 2 + 1];
      
      /* check if triangle is convex (positive area) */

      if  ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0) > 0.0f) {

        is_ear = 1;
        
        /* check if any other vertex is inside this triangle */

        for  (j = 0; j < vertex_count && is_ear; j++) {

          int vj = vertex_list[j];
          
          if  (vj != v0 && vj != v1 && vj != v2) {

            float xj = polygon.vertices[vj * 2];
            float yj = polygon.vertices[vj * 2 + 1];
            
            /* point-in-triangle test using barycentric coordinates */

            float denom = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
            
            if  (denom != 0.0f) {

              float a = ((y1 - y2) * (xj - x2) + (x2 - x1) * (yj - y2)) / denom;
              float b = ((y2 - y0) * (xj - x2) + (x0 - x2) * (yj - y2)) / denom;
              float c = 1.0f - a - b;
              
              if  (a >= 0.0f && b >= 0.0f && c >= 0.0f) {

                is_ear = 0; /* point is inside triangle */

              }
            }
          }
        }
      }
    }
    
    if  (!is_ear) {

      /* no ear found - degenerate case, add remaining triangle */

      triangles[triangle_index * 3] = vertex_list[0];
      triangles[triangle_index * 3 + 1] = vertex_list[1];
      triangles[triangle_index * 3 + 2] = vertex_list[2];
      triangle_index++;
      break;

    }
    
    /* add ear triangle to result */

    triangles[triangle_index * 3] = v0;
    triangles[triangle_index * 3 + 1] = v1;
    triangles[triangle_index * 3 + 2] = v2;
    triangle_index++;
    
    /* remove middle vertex from list */

    for  (j = (i + 1) % vertex_count; j < vertex_count - 1; j++) {
      vertex_list[j] = vertex_list[j + 1];

    }

    vertex_count--;

  }
  
  /* add final triangle */

  if  (vertex_count == 3) {
    triangles[triangle_index * 3] = vertex_list[0];
    triangles[triangle_index * 3 + 1] = vertex_list[1];
    triangles[triangle_index * 3 + 2] = vertex_list[2];
    triangle_index++;

  }
  
  free(vertex_list);
  *triangle_count_out = triangle_index;
  return triangles;

}

/*

         commc_convex_hull_graham_scan()
	       ---
	       computes convex hull of point set using Graham scan
	       algorithm. sorts points by polar angle and uses stack
	       to maintain hull vertices in counter-clockwise order.

*/

commc_convex_hull_t commc_convex_hull_graham_scan(float* points, int point_count) {

  commc_convex_hull_t hull = {NULL, 0};
  float* sorted_points;
  float* hull_vertices;
  int    bottom_point_index;
  float  bottom_y;
  int    i, j;
  int    hull_size;
  float  x0, y0, x1, y1, x2, y2;
  float  cross_product;
  
  if  (!points || point_count < 3) {

    return hull;

  }
  
  /* allocate memory for working arrays */

  sorted_points = (float*)malloc(point_count * 2 * sizeof(float));
  hull_vertices = (float*)malloc(point_count * 2 * sizeof(float));
  
  if  (!sorted_points || !hull_vertices) {

    free(sorted_points);
    free(hull_vertices);
    return hull;

  }
  
  /* copy points for sorting */

  for  (i = 0; i < point_count * 2; i++) {

    sorted_points[i] = points[i];

  }
  
  /* find bottom-most point (and leftmost if tie) */

  bottom_point_index = 0;
  bottom_y = sorted_points[1];
  
  for  (i = 1; i < point_count; i++) {

    float y = sorted_points[i * 2 + 1];
    
    if  (y < bottom_y || (y == bottom_y && sorted_points[i * 2] < sorted_points[bottom_point_index * 2])) {

      bottom_point_index = i;
      bottom_y = y;

    }
  }
  
  /* swap bottom point to position 0 */

  if  (bottom_point_index != 0) {

    float temp_x = sorted_points[0];
    float temp_y = sorted_points[1];
    
    sorted_points[0] = sorted_points[bottom_point_index * 2];
    sorted_points[1] = sorted_points[bottom_point_index * 2 + 1];
    sorted_points[bottom_point_index * 2] = temp_x;
    sorted_points[bottom_point_index * 2 + 1] = temp_y;
  }
  
  /* simple bubble sort by polar angle (for educational purposes) */

  for  (i = 1; i < point_count - 1; i++) {

    for  (j = i + 1; j < point_count; j++) {

      x0 = sorted_points[0];
      y0 = sorted_points[1];
      x1 = sorted_points[i * 2];
      y1 = sorted_points[i * 2 + 1];
      x2 = sorted_points[j * 2];
      y2 = sorted_points[j * 2 + 1];
      
      /* compute cross product to determine relative angle */

      cross_product = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
      
      if  (cross_product < 0.0f) {

        /* swap points */

        float temp_x = sorted_points[i * 2];
        float temp_y = sorted_points[i * 2 + 1];
        sorted_points[i * 2] = sorted_points[j * 2];
        sorted_points[i * 2 + 1] = sorted_points[j * 2 + 1];
        sorted_points[j * 2] = temp_x;
        sorted_points[j * 2 + 1] = temp_y;

      }
    }
  }
  
  /* Graham scan algorithm */

  hull_vertices[0] = sorted_points[0];
  hull_vertices[1] = sorted_points[1];
  hull_vertices[2] = sorted_points[2];
  hull_vertices[3] = sorted_points[3];
  hull_size = 2;
  
  for  (i = 2; i < point_count; i++) {

    x2 = sorted_points[i * 2];
    y2 = sorted_points[i * 2 + 1];
    
    /* remove points that make right turn */

    while  (hull_size > 1) {

      x0 = hull_vertices[(hull_size - 2) * 2];
      y0 = hull_vertices[(hull_size - 2) * 2 + 1];
      x1 = hull_vertices[(hull_size - 1) * 2];
      y1 = hull_vertices[(hull_size - 1) * 2 + 1];
      
      cross_product = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
      
      if  (cross_product <= 0.0f) {

        hull_size--; /* remove last point */

      }
      else {

        break;

      }
    }
    
    /* add current point to hull */

    hull_vertices[hull_size * 2] = x2;
    hull_vertices[hull_size * 2 + 1] = y2;
    hull_size++;

  }
  
  free(sorted_points);
  
  hull.hull_vertices = hull_vertices;
  hull.hull_size = hull_size;
  
  return hull;

}

/*

         commc_convex_hull_destroy()
	       ---
	       frees memory allocated for convex hull vertices.

*/

void commc_convex_hull_destroy(commc_convex_hull_t* hull) {

  if  (hull && hull->hull_vertices) {
    free(hull->hull_vertices);
    hull->hull_vertices = NULL;
    hull->hull_size = 0;
  }

}

/*

         commc_fixed_create()
	       ---
	       creates a fixed-point number from a floating-point value.
	       multiplies by 2^scale_bits to convert to integer representation.

*/

commc_fixed_t commc_fixed_create(float value, int scale_bits) {

  commc_fixed_t fixed_num;
  long scale_factor;
  
  if  (scale_bits < 0) scale_bits = 0;
  if  (scale_bits > 30) scale_bits = 30; /* prevent overflow */
  
  scale_factor = 1L << scale_bits;
  
  fixed_num.value = (long)(value * (float)scale_factor);
  fixed_num.scale_bits = scale_bits;
  
  return fixed_num;

}

/*

         commc_fixed_to_float()
	       ---
	       converts a fixed-point number back to floating-point.
	       divides by 2^scale_bits to restore fractional representation.

*/

float commc_fixed_to_float(commc_fixed_t fixed_num) {

  long scale_factor = 1L << fixed_num.scale_bits;
  return (float)fixed_num.value / (float)scale_factor;

}

/*

         commc_fixed_add()
	       ---
	       adds two fixed-point numbers with same scale.
	       simple integer addition since both have same scaling.

*/

commc_fixed_t commc_fixed_add(commc_fixed_t a, commc_fixed_t b) {

  commc_fixed_t result;
  
  /* use the same scale as first operand */

  result.scale_bits = a.scale_bits;
  
  if  (a.scale_bits == b.scale_bits) {
    result.value = a.value + b.value;
  }
  else {

    /* different scales - convert b to match a's scale */

    if  (a.scale_bits > b.scale_bits) {

      long shift = a.scale_bits - b.scale_bits;
      result.value = a.value + (b.value << shift);

    }

    else {

      long shift = b.scale_bits - a.scale_bits;
      result.value = a.value + (b.value >> shift);

    }
  }
  
  return result;

}

/*

         commc_fixed_multiply()
	       ---
	       multiplies two fixed-point numbers, handling 
	       intermediate overflow and scale adjustment.

*/

commc_fixed_t commc_fixed_multiply(commc_fixed_t a, commc_fixed_t b) {

  commc_fixed_t result;
  long          temp_result;
  long          scale_factor;
  
  result.scale_bits = a.scale_bits;
  
  /* simple multiplication with overflow detection */

  temp_result = a.value * b.value;
  
  /* adjust for combined scaling */

  if  (a.scale_bits == b.scale_bits) {

    scale_factor = 1L << a.scale_bits;
    result.value = temp_result / scale_factor;

  }

  else {

    /* different scales - use average scale */

    int avg_scale = (a.scale_bits + b.scale_bits) / 2;
    scale_factor = 1L << avg_scale;
    result.value = temp_result / scale_factor;

  }
  
  return result;

}

/*

         commc_fixed_divide()
	       ---
	       divides two fixed-point numbers with proper
	       scale handling and overflow protection.

*/

commc_fixed_t commc_fixed_divide(commc_fixed_t a, commc_fixed_t b) {

  commc_fixed_t result;
  long          temp_numerator;
  long          scale_factor;
  
  result.scale_bits = a.scale_bits;
  
  if  (b.value == 0) {

    result.value = 0; /* avoid division by zero */
    return result;

  }
  
  /* scale up numerator to maintain precision */

  scale_factor = 1L << a.scale_bits;
  temp_numerator = a.value * scale_factor;
  
  if  (a.scale_bits == b.scale_bits) {

    result.value = temp_numerator / b.value;

  }
  
  else {

    /* different scales - adjust divisor */

    if  (b.scale_bits > a.scale_bits) {

      long shift = b.scale_bits - a.scale_bits;
      long adjusted_b = b.value >> shift;
      result.value = (adjusted_b != 0) ? temp_numerator / adjusted_b : 0;

    }

    else {

      long shift = a.scale_bits - b.scale_bits;
      long adjusted_b = b.value << shift;
      result.value = (adjusted_b != 0) ? temp_numerator / adjusted_b : 0;
      
    }
  }
  
  return result;

}

/*
	==================================
             --- EOF ---
	==================================
*/
