;lg_math_vector.asm - 3D math routines for the Legacy 3D Engine.
;Vector operations.
;Copyright (c) 2006, Blaine Myers
;
;All functions are __cdecl calling convention (see lm_math.h).

[BITS 32] ;Set 32 bit code generation.

[SECTION .code] ;Section containing code.

;L_vector3* L_vec3add(L_vector3* pOut, L_vector3* pV1, L_vector3* pV2)
;
;Adds two 3 component vectors together.
global _L_vec3add
_L_vec3add:
	mov  eax,  [esp+4] ;Get pointers to the parameters.
	mov  ecx,  [esp+8]
	mov  edx,  [esp+12]
	fld  dword [ecx]   ;Then add the x, y, and z component respectively.
   fadd dword [edx]
	fstp dword [eax]
	fld  dword [ecx+4]
	fadd dword [edx+4]
	fstp dword [eax+4]
	fld  dword [ecx+8]
	fadd dword [edx+8]
	fstp dword [eax+8] 
	ret                ;The return value of the output vector is already in eax.

;L_vecgtor3* L_vec3cross(L_vector3* pOut, L_vector3* pV1, L_vector3* pV2)
;
;Finds the cross product of (*pV1)X(*pV2).
global _L_vec3cross
_L_vec3cross:
	mov eax, [esp+4]   ;Save pointers to parameters,
	mov ecx, [esp+8]   ;and make room on stack for output.
	mov edx, [esp+12]
	sub esp, 12

	fld  dword [ecx+8] ;Calculate the cross value for x,
	fmul dword [edx+4] ;store at [esp].
	fld  dword [ecx+4]
	fmul dword [edx+8]
	fsub st1
	fstp dword [esp]

	fld  dword [ecx]   ;Calculate the cross value for y,
	fmul dword [edx+8] ;and store t [esp+4].
	fld  dword [ecx+8]
	fmul dword [edx]
	fsub st1
	fstp dword [esp+4]

	fld  dword [ecx+4] ;Calculate teh cross value for z,
	fmul dword [edx]   ;and store at [esp+8].
	fld  dword [ecx]
	fmul dword [edx+4]
	fsub st1
	fstp dword [esp+8]

	pop dword [eax]    ;Pop output values off the stack, into pOut.
	pop dword [eax+4]
	pop dword [eax+8]  ;pOut is already set to eax as the return value.
	ret

;float L_vec3dot(L_vector3* pV1, L_vector3* pV2)
;
;Finds the dot product of (*pV1) * (*pV2).
global _L_vec3dot
_L_vec3dot:
	mov ecx, [esp+4]
	mov edx, [esp+8]

	fld   dword [ecx]   ;x1*x2+
	fmul  dword [edx]
	fld   dword [ecx+4] ;y1*y2+
	fmul  dword [edx+4]
	faddp st1
	fld   dword [ecx+8] ;z1*z2
	fmul  dword [edx+8]
	faddp st1
	ret                 ;result is in st0.


;float L_vec3length(L_vector3* pV)
;
;Finds the length of the given vector.
global _L_vec3length
_L_vec3length:
	mov ecx, [esp+4]
	L_vec3getlength:
	fld   dword [ecx]
	fmul  dword [ecx]
	fld   dword [ecx+4]
	fmul  dword [ecx+4]
	faddp st1
	fld   dword [ecx+8]
	fmul  dword [ecx+8]
	faddp st1
	fsqrt
	ret

;L_vector3* L_vec3normalize(L_vector3* pOut, L_vector3* pV);
;
;Returns a normalized vector with the same direction as pV.
global _L_vec3normalize
_L_vec3normalize:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	sub esp, 4

	;Find the magnitude (length) of the vector.
	call L_vec3getlength ;Calling L_vec3getlength (as opposed to _L_vec3length) will return the
	                     ;length, of the vector at ecx in st0.

	;Find the ratio (1/magnitude).
	mov   dword [esp], 3F800000H
	fld   dword [esp]
	fxch  st0, st1
	fdivp st1

	;Scale the vector by the ratio we found.
	call L_vec3doscale
	add  esp, 4
	ret


;L_vector3* L_vec3subtract(L_vector3* pOut, L_vector3* pV1, L_vector3* pV2);
;
;Finds the difference between two vectors. pV1-pV2.
global _L_vec3subtract
_L_vec3subtract:
	mov  eax,  [esp+4] ;Get pointers to the parameters.
	mov  ecx,  [esp+8]
	mov  edx,  [esp+12]
	fld  dword [ecx]   ;Then subtract the x, y, and z component respectively.
   fsub dword [edx]
	fstp dword [eax]
	fld  dword [ecx+4]
	fsub dword [edx+4]
	fstp dword [eax+4]
	fld  dword [ecx+8]
	fsub dword [edx+8]
	fstp dword [eax+8] 
	ret                ;The return value of the output vector is already in eax.

;L_vector3* L_vec3scale(L_vector3* pOut, L_vector3* pV, float s).
;
;Scales teh given vector by s, part of this function is used
;by L_vec3normalize, to scale to a normal vector (L_vec3doscale).
global _L_vec3scale
_L_vec3scale:
	mov  eax,  [esp+4]   ;Get pointers to the parameters.
	mov  ecx,  [esp+8]

	fld   dword [esp+12] ;Multiply x, y, and z by scaler [esp+12].
	L_vec3doscale:
	fld   dword [ecx]
	fmul  st1
	fstp  dword [eax]
	fld   dword [ecx+4]
	fmul  st1
	fstp  dword [eax+4]
	fld   dword [ecx+8]
	fmulp st1           ;Pop the scaler off the stack this time.
	fstp  dword [eax+8]
	ret                 ;result is already in eax.   

;float L_vec3distance(L_vector3* pV1, L_vector3* pV2)
;
;Finds the distance between two vectors.
global _L_vec3distance
_L_vec3distance:
	mov   ecx, [esp+4]  ;Use the pythagoram theorem to calculate the distance between two vecotrs.
	mov   edx, [esp+8]
	fld   dword [ecx]   ;Get (v1.x-v2.x)^2
	fsub  dword [edx]
	fld   st0
	fmulp st1
	fld   dword [ecx+4] ;Get (v1.y-v1.y)^2
	fsub  dword [edx+4]
	fld   st0
	fmulp st1
	faddp st1           ;Add to x^2
	fld   dword [ecx+8] ;Get (v1.z-v2.z)^2
	fsub  dword [edx+8]
	fld   st0
	fmulp st1
	faddp st1           ;Add to x^2+y^2
	fsqrt               ;Take the sqrt, and the value is saved in st0, which is the return value.
	ret
