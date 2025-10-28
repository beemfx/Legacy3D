#ifndef __ML_TYPES_H__
#define __ML_TYPES_H__


#ifdef ML_LIB_BYTEPACK
#include <pshpack1.h>
#endif ML_LIB_BYTEPACK

/* Some types that we will use.*/
typedef unsigned char  ml_byte;
typedef signed char    ml_sbyte;
typedef unsigned short ml_word;
typedef signed short   ml_short;
typedef unsigned long  ml_dword;
typedef signed long    ml_long;
typedef int            ml_bool;
typedef unsigned int   ml_uint;
typedef signed int     ml_int;
typedef void           ml_void;
typedef float          ml_float;

/* A few definitions. */
#define ML_TRUE  (1)
#define ML_FALSE (0)

#define ML_Max(v1, v2) ((v1)>(v2))?(v1):(v2)
#define ML_Min(v1, v2) ((v1)<(v2))?(v1):(v2)
#define ML_Clamp(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )

#define ML_FUNC __cdecl



/****************************************
	Some constants for the math library.
****************************************/

#define ML_PI     ((ml_float)3.141592654f)
#define ML_2PI    ((ml_float)3.141592654f*2.0f)
#define ML_HALFPI ((ml_float)3.141592654f*0.5f)


/*************************************
	Structures for the math functions. 
*************************************/
typedef struct _ML_MATRIX{
	ml_float _11, _12, _13, _14;
	ml_float _21, _22, _23, _24;
	ml_float _31, _32, _33, _34;
	ml_float _41, _42, _43, _44;
}ML_MATRIX, ML_MAT, ml_mat;

typedef struct _ML_QUATERNION{
	ml_float x, y, z, w;
}ML_QUATERNION, ML_QUAT, ml_quat;

typedef struct _ML_VECTOR2{
	ml_float x, y;
}ML_VECTOR2, ML_VEC2, ml_vec2;

typedef struct _ML_VECTOR3{
	ml_float x, y, z;
}ML_VECTOR3, ML_VEC3, ml_vec3;

typedef struct _ML_VECTOR4{
	ml_float x, y, z, w;
}LML_VECTOR4, ML_VEC4, ml_vec4;

typedef struct _ML_PLANE{
	ml_float a, b, c, d;
}ML_PLANE, ml_plane;

typedef struct _ML_AABB{
	ML_VEC3 v3Min;
	ML_VEC3 v3Max;
}ML_AABB, ml_aabb;

#ifdef ML_LIB_BYTEPACK
#include <poppack.h>
#endif ML_LIB_BYTEPACK

#endif __ML_TYPES_H__