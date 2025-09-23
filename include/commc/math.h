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

#ifndef   COMMC_MATH_H
#define   COMMC_MATH_H

#include  <math.h> 								  /* for sin, cos, sqrt, tan */

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

/* 3x3 matrix (row-major for easy indexing). */

typedef struct {

  float m[9]; 								/* 3x3 matrix elements */

} commc_mat3_t;

/* quaternion for rotations. */

typedef struct {

  float x;
  float y;
  float z;
  float w;

} commc_quat_t;

/* LU decomposition result for 4x4 matrices with partial pivoting. */

typedef struct {

  commc_mat4_t lower;               /* lower triangular matrix L */
  commc_mat4_t upper;               /* upper triangular matrix U */
  int          pivots[4];           /* permutation indices */
  int          is_singular;         /* 1 if matrix is singular, 0 otherwise */

} commc_mat4_lu_t;

/* complex number for eigenvalue computation. */

typedef struct {

  float real;                       /* real part */
  float imag;                       /* imaginary part */

} commc_complex_t;

/* eigenvalue computation result for 4x4 matrices. */

typedef struct {

  commc_complex_t eigenvalues[4];   /* up to 4 eigenvalues (real + imaginary) */
  commc_vec3_t    eigenvectors[4];  /* corresponding eigenvectors (real part only) */
  int             num_real;         /* number of real eigenvalues found */
  int             converged;        /* 1 if iteration converged, 0 otherwise */
  int             iterations;       /* number of iterations performed */

} commc_mat4_eigen_t;

/* control point for cubic spline interpolation. */

typedef struct {

  float x;                          /* parameter value (time/position) */
  float y;                          /* function value at this point */

} commc_spline_point_t;

/* cubic spline segment coefficients (ax^3 + bx^2 + cx + d). */

typedef struct {

  float a;                          /* cubic coefficient */
  float b;                          /* quadratic coefficient */
  float c;                          /* linear coefficient */
  float d;                          /* constant coefficient */
  float x_start;                    /* start parameter for this segment */
  float x_end;                      /* end parameter for this segment */

} commc_spline_segment_t;

/* cubic spline interpolation data structure. */

typedef struct {

  commc_spline_point_t*   points;           /* control points array */
  commc_spline_segment_t* segments;         /* computed spline segments */
  int                     num_points;       /* number of control points */
  int                     num_segments;     /* number of spline segments */
  int                     is_computed;      /* 1 if spline coefficients computed */

} commc_cubic_spline_t;

/* control point for Bezier curves. */

typedef struct {

  float x;                          /* x coordinate */
  float y;                          /* y coordinate */

} commc_bezier_point_t;

/* quadratic Bezier curve (3 control points). */

typedef struct {

  commc_bezier_point_t p0;          /* start point */
  commc_bezier_point_t p1;          /* control point */
  commc_bezier_point_t p2;          /* end point */

} commc_bezier_quadratic_t;

/* cubic Bezier curve (4 control points). */

typedef struct {

  commc_bezier_point_t p0;          /* start point */
  commc_bezier_point_t p1;          /* first control point */
  commc_bezier_point_t p2;          /* second control point */
  commc_bezier_point_t p3;          /* end point */

} commc_bezier_cubic_t;

/*

         commc_bspline_t
	       ---
	       B-spline curve with control points, knot vector, and degree.
	       supports both rational (NURBS) and non-rational B-splines.
	       the knot vector must be non-decreasing with size 
	       (num_points + degree + 1).

*/

typedef struct {

  commc_bezier_point_t*  control_points;    /* ARRAY OF CONTROL POINTS */
  float*                 knots;             /* KNOT VECTOR */
  float*                 weights;           /* POINT WEIGHTS (NULL FOR NON-RATIONAL) */
  int                    num_points;        /* NUMBER OF CONTROL POINTS */
  int                    num_knots;         /* NUMBER OF KNOTS */
  int                    degree;            /* SPLINE DEGREE */

} commc_bspline_t;

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

         commc_mat4_lu_decompose()
	       ---
	       performs LU decomposition with partial pivoting on a 4x4 matrix.
	       decomposes matrix A into PA = LU where P is permutation matrix,
	       L is lower triangular, and U is upper triangular.
	       
	       this is essential for solving linear systems Ax = b efficiently,
	       computing determinants, and matrix inversion. partial pivoting
	       improves numerical stability by reducing round-off errors.

*/

commc_mat4_lu_t commc_mat4_lu_decompose(commc_mat4_t matrix);

/*

         commc_mat4_lu_solve()
	       ---
	       solves the linear system Ax = b using precomputed LU decomposition.
	       uses forward substitution (Ly = Pb) followed by back substitution
	       (Ux = y) to find the solution vector x efficiently.
	       
	       this is much faster than matrix inversion for solving multiple
	       systems with the same coefficient matrix A.

*/

commc_vec3_t commc_mat4_lu_solve(commc_mat4_lu_t lu_decomp, commc_vec3_t b);

/*

         commc_mat4_lu_determinant()
	       ---
	       computes the determinant of a matrix using its LU decomposition.
	       det(A) = det(P) * det(L) * det(U) = sign(P) * prod(U_diagonal)
	       where sign(P) accounts for row swaps during pivoting.
	       
	       this is more numerically stable and efficient than computing
	       determinant directly using cofactor expansion.

*/

float commc_mat4_lu_determinant(commc_mat4_lu_t lu_decomp);

/*

         commc_mat4_eigenvalue_dominant()
	       ---
	       finds the dominant (largest magnitude) eigenvalue using power iteration.
	       this is the simplest eigenvalue algorithm, repeatedly multiplying
	       a vector by the matrix until it converges to the eigenvector
	       corresponding to the largest eigenvalue.
	       
	       power iteration is guaranteed to converge for matrices with
	       a unique dominant eigenvalue, making it ideal for many practical
	       applications like PageRank and principal component analysis.

*/

commc_complex_t commc_mat4_eigenvalue_dominant(commc_mat4_t matrix, int max_iterations);

/*

         commc_mat4_eigenvector_dominant()
	       ---
	       finds the eigenvector corresponding to the dominant eigenvalue.
	       uses power iteration with the same convergence criteria as
	       eigenvalue computation, returning the normalized eigenvector.
	       
	       the eigenvector represents the direction of greatest variance
	       or the principal axis of transformation for the matrix.

*/

commc_vec3_t commc_mat4_eigenvector_dominant(commc_mat4_t matrix, int max_iterations);

/*

         commc_mat4_eigen_decompose()
	       ---
	       computes eigenvalues and eigenvectors using QR decomposition.
	       attempts to find all eigenvalues of the matrix, including
	       complex ones, using the iterative QR algorithm.
	       
	       this is more computationally intensive than power iteration
	       but provides a complete eigendecomposition. complex eigenvalues
	       are stored as (real, imaginary) pairs in the result structure.

*/

commc_mat4_eigen_t commc_mat4_eigen_decompose(commc_mat4_t matrix, int max_iterations);

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

         commc_quat_to_mat3()
	       ---
	       converts a quaternion to a 3x3 rotation matrix.
	       this is useful when you only need the rotational part
	       without translation or homogeneous coordinates, providing
	       a more compact representation for pure rotations.

*/

commc_mat3_t commc_quat_to_mat3(commc_quat_t q);

/*

         commc_mat4_to_quat()
	       ---
	       converts a 4x4 rotation matrix to a quaternion.
	       extracts the rotation component from a transformation matrix
	       and converts it to quaternion representation for efficient
	       rotation interpolation and composition.
	       
	       assumes the matrix represents a pure rotation (orthogonal with
	       determinant 1) for accurate conversion.

*/

commc_quat_t commc_mat4_to_quat(commc_mat4_t matrix);

/*

         commc_mat3_to_quat()
	       ---
	       converts a 3x3 rotation matrix to a quaternion.
	       more direct conversion from pure rotation matrices
	       without extracting from 4x4 transformation matrices.
	       
	       uses Shepperd's method for numerical stability when
	       converting rotation matrices to quaternion form.

*/

commc_quat_t commc_mat3_to_quat(commc_mat3_t matrix);

/*

         commc_quat_dot()
	       ---
	       computes the dot product of two quaternions.
	       essential for SLERP interpolation to determine the
	       angle between quaternions and detect when to use
	       the shortest rotation path.

*/

float commc_quat_dot(commc_quat_t q1, commc_quat_t q2);

/*

         commc_quat_slerp()
	       ---
	       performs spherical linear interpolation between two quaternions.
	       provides smooth rotation interpolation with constant angular velocity,
	       essential for animation systems and camera controls.
	       
	       parameter t controls interpolation: 0.0 returns q1, 1.0 returns q2.
	       automatically selects the shortest rotation path to avoid unnecessary
	       spinning during interpolation.

*/

commc_quat_t commc_quat_slerp(commc_quat_t q1, commc_quat_t q2, float t);

/*

         commc_quat_nlerp()
	       ---
	       performs normalized linear interpolation between two quaternions.
	       faster alternative to SLERP that provides acceptable results for
	       small angular differences, commonly used in real-time applications
	       where performance is more critical than perfect interpolation.

*/

commc_quat_t commc_quat_nlerp(commc_quat_t q1, commc_quat_t q2, float t);

/*

         commc_cubic_spline_create()
	       ---
	       creates a new cubic spline from an array of control points.
	       allocates memory for the spline structure and copies the
	       control points. call commc_cubic_spline_compute() to
	       calculate spline coefficients before interpolation.

*/

commc_cubic_spline_t* commc_cubic_spline_create(commc_spline_point_t* points, int num_points);

/*

         commc_cubic_spline_compute()
	       ---
	       computes cubic spline coefficients using natural boundary
	       conditions (second derivative is zero at endpoints).
	       this sets up the spline segments for smooth interpolation
	       between the control points.

*/

int commc_cubic_spline_compute(commc_cubic_spline_t* spline);

/*

         commc_cubic_spline_interpolate()
	       ---
	       evaluates the cubic spline at parameter value x.
	       returns the interpolated function value using the
	       computed spline coefficients. x should be within
	       the range of the control points for valid results.

*/

float commc_cubic_spline_interpolate(commc_cubic_spline_t* spline, float x);

/*

         commc_cubic_spline_derivative()
	       ---
	       computes the first derivative of the cubic spline at x.
	       useful for calculating tangent vectors and velocities
	       along the interpolated curve.

*/

float commc_cubic_spline_derivative(commc_cubic_spline_t* spline, float x);

/*

         commc_cubic_spline_destroy()
	       ---
	       releases memory allocated for the cubic spline structure.
	       always call this when done with a spline to prevent
	       memory leaks.

*/

void commc_cubic_spline_destroy(commc_cubic_spline_t* spline);

/*

         commc_bezier_linear()
	       ---
	       evaluates a linear Bezier curve (simple linear interpolation).
	       parameter t ranges from 0.0 to 1.0, where 0.0 returns p0
	       and 1.0 returns p1. equivalent to linear interpolation
	       between two points.

*/

commc_bezier_point_t commc_bezier_linear(commc_bezier_point_t p0, commc_bezier_point_t p1, float t);

/*

         commc_bezier_quadratic()
	       ---
	       evaluates a quadratic Bezier curve using three control points.
	       uses de casteljau's algorithm for numerically stable evaluation.
	       parameter t ranges from 0.0 to 1.0, creating smooth curved
	       paths between start and end points influenced by control point.

*/

commc_bezier_point_t commc_bezier_quadratic(commc_bezier_quadratic_t curve, float t);

/*

         commc_bezier_cubic()
	       ---
	       evaluates a cubic Bezier curve using four control points.
	       provides maximum flexibility for smooth curve design with
	       two control points influencing the curve shape between
	       start and end points. fundamental for graphics and animation.

*/

commc_bezier_point_t commc_bezier_cubic(commc_bezier_cubic_t curve, float t);

/*

         commc_bezier_quadratic_derivative()
	       ---
	       computes the first derivative (tangent vector) of a quadratic
	       Bezier curve at parameter t. useful for calculating velocities
	       and orientations along the curve path.

*/

commc_bezier_point_t commc_bezier_quadratic_derivative(commc_bezier_quadratic_t curve, float t);

/*

         commc_bezier_cubic_derivative()
	       ---
	       computes the first derivative (tangent vector) of a cubic
	       Bezier curve at parameter t. essential for animation systems
	       requiring smooth velocity calculations along curved paths.

*/

commc_bezier_point_t commc_bezier_cubic_derivative(commc_bezier_cubic_t curve, float t);

/*

         commc_bezier_cubic_length()
	       ---
	       estimates the arc length of a cubic Bezier curve using
	       adaptive quadrature. provides approximate curve length
	       for arc-length parameterization and animation timing.

*/

float commc_bezier_cubic_length(commc_bezier_cubic_t curve, int subdivisions);

/*

         commc_bspline_evaluate()
	       ---
	       evaluates a B-spline curve at parameter u using the
	       Cox-de Boor recursion formula. supports both rational
	       and non-rational B-splines of arbitrary degree.

*/

commc_bezier_point_t commc_bspline_evaluate(commc_bspline_t spline, float u);

/*

         commc_bspline_derivative()
	       ---
	       computes the first derivative of a B-spline curve at
	       parameter u. uses the derivative formula for B-splines
	       to compute tangent vectors along the curve.

*/

commc_bezier_point_t commc_bspline_derivative(commc_bspline_t spline, float u);

/*

         commc_bspline_find_span()
	       ---
	       finds the knot span index for parameter u in the knot vector.
	       essential utility function for B-spline evaluation algorithms.

*/

int commc_bspline_find_span(commc_bspline_t spline, float u);

/*

         commc_bspline_basis()
	       ---
	       computes all non-zero basis functions for a given span and
	       parameter using the Cox-de Boor recursion. fundamental
	       building block for B-spline evaluation.

*/

void commc_bspline_basis(commc_bspline_t spline, int span, float u, float* basis);

/*

         commc_perlin_noise()
	       ---
	       generates Perlin noise at the given coordinates using
	       Ken Perlin's improved noise algorithm. produces smooth,
	       pseudo-random values suitable for procedural generation.

*/

float commc_perlin_noise(float x, float y, float z);

/*

         commc_perlin_noise_2d()
	       ---
	       generates 2D Perlin noise for applications that only
	       require two dimensions. optimized version that reduces
	       computation overhead for 2D use cases.

*/

float commc_perlin_noise_2d(float x, float y);

/*

         commc_perlin_seed()
	       ---
	       seeds the Perlin noise generator with a custom permutation
	       table. allows reproducible noise patterns for consistent
	       procedural generation across runs.

*/

void commc_perlin_seed(unsigned int seed);

/*

         commc_simplex_noise()
	       ---
	       generates simplex noise at the given coordinates using
	       Ken Perlin's simplex noise algorithm. more efficient than
	       Perlin noise with fewer directional artifacts.

*/

float commc_simplex_noise(float x, float y);

/*

         commc_simplex_noise_3d()
	       ---
	       generates 3D simplex noise for volumetric applications.
	       uses tetrahedral lattice for improved isotropy compared
	       to cubic lattice Perlin noise.

*/

float commc_simplex_noise_3d(float x, float y, float z);

/*

         commc_fractal_noise()
	       ---
	       generates fractal noise by layering multiple octaves of
	       Perlin noise with varying frequency and amplitude.
	       creates natural-looking textures and terrain.

*/

float commc_fractal_noise(float x, float y, int octaves, float persistence, float lacunarity);

/*

         commc_fractal_noise_3d()
	       ---
	       generates 3D fractal noise for volumetric applications.
	       combines multiple octaves of 3D Perlin noise for rich
	       procedural textures and volumetric effects.

*/

float commc_fractal_noise_3d(float x, float y, float z, int octaves, float persistence, float lacunarity);

/*

         commc_ridged_noise()
	       ---
	       generates ridged fractal noise by taking the absolute
	       value and inverting standard fractal noise. produces
	       ridge-like features useful for mountain terrain.

*/

float commc_ridged_noise(float x, float y, int octaves, float persistence, float lacunarity);

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
	==================================
        --- NUMERICAL ANALYSIS ---
	==================================
*/

/* function pointer type for functions to be integrated or analyzed. */

typedef float (*commc_function_t)(float x);
typedef float (*commc_function_with_derivative_t)(float x, float* derivative_out);

/* structure for numerical integration results. */

typedef struct {
  
  float result;           /* COMPUTED INTEGRAL VALUE */
  float error_estimate;   /* ERROR BOUND OR ESTIMATE */
  int   evaluations;      /* NUMBER OF FUNCTION EVALUATIONS */
  int   success;          /* 1 IF SUCCESSFUL, 0 IF FAILED */
  
} commc_integration_result_t;

/* structure for root finding results. */

typedef struct {
  
  float root;             /* FOUND ROOT VALUE */
  float error;            /* ERROR IN ROOT */
  int   iterations;       /* NUMBER OF ITERATIONS */
  int   success;          /* 1 IF ROOT FOUND, 0 IF FAILED */
  
} commc_root_result_t;

/* structure for optimization results. */

typedef struct {
  
  float x_optimal;        /* OPTIMAL X VALUE */
  float f_optimal;        /* FUNCTION VALUE AT OPTIMUM */
  int   iterations;       /* NUMBER OF ITERATIONS */
  int   success;          /* 1 IF CONVERGED, 0 IF FAILED */
  
} commc_optimization_result_t;

/*

         commc_integrate_trapezoidal()
	       ---
	       performs numerical integration using the trapezoidal rule.
	       divides the interval [a, b] into n equal subintervals and
	       approximates the area under f(x) using trapezoids.
	       
	       provides O(h^2) accuracy where h is the step size.

*/

commc_integration_result_t commc_integrate_trapezoidal(commc_function_t f, float a, float b, int n);

/*

         commc_integrate_simpson()
	       ---
	       performs numerical integration using Simpson's 1/3 rule.
	       uses parabolic approximations to achieve higher accuracy
	       than the trapezoidal rule, with O(h^4) error.
	       
	       requires an even number of intervals.

*/

commc_integration_result_t commc_integrate_simpson(commc_function_t f, float a, float b, int n);

/*

         commc_integrate_adaptive()
	       ---
	       performs adaptive quadrature integration using recursive
	       Simpson's rule with error control. automatically subdivides
	       intervals where error is too large.
	       
	       continues until desired tolerance is achieved.

*/

commc_integration_result_t commc_integrate_adaptive(commc_function_t f, float a, float b, float tolerance);

/*

         commc_root_bisection()
	       ---
	       finds a root of f(x) = 0 using the bisection method.
	       requires that f(a) and f(b) have different signs.
	       converges slowly but reliably to a root.
	       
	       guaranteed convergence if root exists in [a, b].

*/

commc_root_result_t commc_root_bisection(commc_function_t f, float a, float b, float tolerance, int max_iterations);

/*

         commc_root_newton_raphson()
	       ---
	       finds a root using Newton-Raphson method with derivative.
	       requires both function and its derivative. converges
	       quadratically when close to root.
	       
	       may fail if derivative is zero or near zero.

*/

commc_root_result_t commc_root_newton_raphson(commc_function_with_derivative_t f, float x0, float tolerance, int max_iterations);

/*

         commc_root_secant()
	       ---
	       finds a root using the secant method. similar to Newton-Raphson
	       but uses numerical derivative approximation instead of
	       analytical derivative.
	       
	       requires two initial guesses x0 and x1.

*/

commc_root_result_t commc_root_secant(commc_function_t f, float x0, float x1, float tolerance, int max_iterations);

/*

         commc_optimize_gradient_descent()
	       ---
	       performs gradient descent optimization to find local minimum
	       of f(x). uses numerical gradient if derivative not provided.
	       learning rate controls step size.
	       
	       may get stuck in local minima.

*/

commc_optimization_result_t commc_optimize_gradient_descent(commc_function_t f, float x0, float learning_rate, float tolerance, int max_iterations);

/*

         commc_optimize_gradient_descent_with_derivative()
	       ---
	       gradient descent optimization using analytical derivative
	       for better accuracy and performance compared to numerical
	       gradient approximation.

*/

commc_optimization_result_t commc_optimize_gradient_descent_with_derivative(commc_function_with_derivative_t f, float x0, float learning_rate, float tolerance, int max_iterations);

/* 
	==================================
          --- STATISTICS ---
	==================================
*/

/* structure for histogram data and analysis. */

typedef struct {
  
  float* bins;            /* ARRAY OF BIN VALUES */
  int    num_bins;        /* NUMBER OF BINS */
  float  min_value;       /* MINIMUM VALUE IN DATASET */
  float  max_value;       /* MAXIMUM VALUE IN DATASET */
  float  bin_width;       /* WIDTH OF EACH BIN */
  int    total_count;     /* TOTAL NUMBER OF DATA POINTS */
  
} commc_histogram_t;

/* structure for basic statistical analysis results. */

typedef struct {
  
  float mean;             /* ARITHMETIC MEAN */
  float median;           /* MEDIAN VALUE */
  float mode;             /* MODE (MOST FREQUENT VALUE) */
  float variance;         /* POPULATION VARIANCE */
  float std_deviation;    /* STANDARD DEVIATION */
  float min_value;        /* MINIMUM VALUE */
  float max_value;        /* MAXIMUM VALUE */
  float range;            /* RANGE (MAX - MIN) */
  float skewness;         /* SKEWNESS (ASYMMETRY) */
  float kurtosis;         /* KURTOSIS (TAIL BEHAVIOR) */
  
} commc_statistics_t;

/*

         commc_random_uniform()
	       ---
	       generates a random number from uniform distribution
	       in the range [min_val, max_val]. every value in
	       the range has equal probability.

*/

float commc_random_uniform(float min_val, float max_val);

/*

         commc_random_normal()
	       ---
	       generates a random number from normal (Gaussian)
	       distribution using Box-Muller transformation.
	       mean specifies center, sigma specifies spread.

*/

float commc_random_normal(float mean, float sigma);

/*

         commc_random_exponential()
	       ---
	       generates a random number from exponential distribution
	       with rate parameter lambda. models time between events
	       in Poisson processes.

*/

float commc_random_exponential(float lambda);

/*

         commc_probability_density_normal()
	       ---
	       computes probability density function for normal
	       distribution at given point x. returns the height
	       of the bell curve at x.

*/

float commc_probability_density_normal(float x, float mean, float sigma);

/*

         commc_cumulative_distribution_normal()
	       ---
	       computes cumulative distribution function for normal
	       distribution. returns probability that random variable
	       is less than or equal to x.

*/

float commc_cumulative_distribution_normal(float x, float mean, float sigma);

/*

         commc_histogram_create()
	       ---
	       creates a histogram from data array by dividing the
	       range [min, max] into num_bins equal-width bins and
	       counting occurrences in each bin.

*/

commc_histogram_t* commc_histogram_create(float* data, int data_size, int num_bins);

/*

         commc_histogram_destroy()
	       ---
	       frees memory allocated for histogram structure
	       and its internal bin array.

*/

void commc_histogram_destroy(commc_histogram_t* histogram);

/*

         commc_histogram_add_value()
	       ---
	       adds a single value to the histogram by finding
	       the appropriate bin and incrementing its count.

*/

void commc_histogram_add_value(commc_histogram_t* histogram, float value);

/*

         commc_histogram_get_percentile()
	       ---
	       computes the value at given percentile (0.0 to 1.0)
	       from histogram data. percentile 0.5 gives median.

*/

float commc_histogram_get_percentile(commc_histogram_t* histogram, float percentile);

/*

         commc_statistics_compute()
	       ---
	       computes comprehensive statistical analysis of data array
	       including mean, median, variance, skewness, and kurtosis.
	       requires sorted data for accurate median computation.

*/

commc_statistics_t commc_statistics_compute(float* data, int data_size);

/*

         commc_statistics_correlation()
	       ---
	       computes Pearson correlation coefficient between two
	       data arrays. returns value between -1 and 1 indicating
	       linear relationship strength.

*/

float commc_statistics_correlation(float* x_data, float* y_data, int data_size);

/*

         commc_statistics_linear_regression()
	       ---
	       performs simple linear regression on (x, y) data pairs.
	       computes slope and intercept for best-fit line.
	       stores results in provided slope and intercept pointers.

*/

void commc_statistics_linear_regression(float* x_data, float* y_data, int data_size, float* slope_out, float* intercept_out);

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
