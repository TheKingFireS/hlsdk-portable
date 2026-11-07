#ifndef HLSDK_CG_COMPAT_CGGL_H
#define HLSDK_CG_COMPAT_CGGL_H
#include "cg.h"
#ifdef __cplusplus
extern "C" {
#endif
CGbool cgGLIsProfileSupported(CGprofile profile);
void cgGLEnableProfile(CGprofile profile);
void cgGLDisableProfile(CGprofile profile);
CGprofile cgGLGetLatestProfile(CGenum profile_type);
void cgGLSetOptimalOptions(CGprofile profile);
void cgGLLoadProgram(CGprogram program);
CGbool cgGLIsProgramLoaded(CGprogram program);
void cgGLBindProgram(CGprogram program);
void cgGLUnbindProgram(CGprofile profile);
void cgGLSetParameter1f(CGparameter parameter, float x);
void cgGLSetParameter4fv(CGparameter parameter, const float *values);
void cgGLSetStateMatrixParameter(CGparameter parameter, int matrix, int transform);
#ifdef __cplusplus
}
#endif
#endif
