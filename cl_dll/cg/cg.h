#ifndef HLSDK_CG_COMPAT_CG_H
#define HLSDK_CG_COMPAT_CG_H
#ifdef __cplusplus
extern "C" {
#endif

typedef int CGbool;
typedef int CGenum;
typedef int CGprofile;
typedef int CGerror;
typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;

#define CG_FALSE 0
#define CG_TRUE 1
#define CG_SOURCE 0
#define CG_PROFILE_UNKNOWN 0
#define CG_PROFILE_VP20 1
#define CG_PROFILE_FP20 2
#define CG_PROFILE_ARBVP1 3
#define CG_PROFILE_ARBFP1 4
#define CG_NO_ERROR 0
#define CG_COMPILER_ERROR 1
#define CG_INVALID_PROFILE_ERROR 2
#define CG_INVALID_PROGRAM_ERROR 3
#define CG_INVALID_PARAMETER_ERROR 4
#define CG_UNKNOWN_ERROR 999
#define CG_GL_VERTEX 8
#define CG_GL_FRAGMENT 9
#define CG_GL_MODELVIEW_MATRIX 4
#define CG_GL_PROJECTION_MATRIX 5
#define CG_GL_TEXTURE_MATRIX 6
#define CG_GL_MODELVIEW_PROJECTION_MATRIX 7

CGcontext cgCreateContext(void);
void cgDestroyContext(CGcontext ctx);
CGbool cgIsContext(CGcontext ctx);
const char *cgGetLastListing(CGcontext ctx);
CGprogram cgCreateProgramFromFile(CGcontext ctx, CGenum program_type, const char *program_file, CGprofile profile, const char *entry, const char **args);
CGprogram cgCreateProgram(CGcontext ctx, CGenum program_type, const char *program, CGprofile profile, const char *entry, const char **args);
void cgDestroyProgram(CGprogram program);
CGbool cgIsProgram(CGprogram program);
void cgCompileProgram(CGprogram program);
CGbool cgIsProgramCompiled(CGprogram program);
CGprofile cgGetProgramProfile(CGprogram program);
CGerror cgGetError(void);
const char *cgGetErrorString(CGerror error);
const char *cgGetLastErrorString(CGerror *error);
CGparameter cgGetNamedParameter(CGprogram program, const char *name);
void cgDestroyParameter(CGparameter parameter);
void cgSetParameter1f(CGparameter parameter, float x);
void cgSetParameter4f(CGparameter parameter, float x, float y, float z, float w);
void cgSetParameter4fv(CGparameter parameter, const float *values);

#ifdef __cplusplus
}
#endif
#endif
