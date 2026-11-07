/*
 * Portable source-compatibility shim for the Cg 1.2 calls used by classic mods.
 * It is linked into client.dll. It deliberately reports shader compilation as
 * unsupported: a real Cg-to-GLSL/nvFX translator is still required to render
 * Cg source files. Never mistake this shim for an implementation of Cg.
 */
#include "cg/cg.h"
#include "cg/cgGL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _CGcontext { CGerror error; char listing[192]; };
struct _CGprogram { CGcontext context; CGprofile profile; int compiled; };
struct _CGparameter { char name[96]; float value[4]; };

static CGerror g_error = CG_NO_ERROR;
static char g_error_text[192] = "No error";
static void fail(CGcontext ctx, CGerror error, const char *text)
{
	g_error = error;
	strncpy(g_error_text, text, sizeof(g_error_text) - 1);
	g_error_text[sizeof(g_error_text) - 1] = '\0';
	if (ctx) {
		ctx->error = error;
		strncpy(ctx->listing, text, sizeof(ctx->listing) - 1);
		ctx->listing[sizeof(ctx->listing) - 1] = '\0';
	}
}

CGcontext cgCreateContext(void)
{
	CGcontext ctx = (CGcontext)calloc(1, sizeof(*ctx));
	if (!ctx) fail(NULL, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory");
	return ctx;
}
void cgDestroyContext(CGcontext ctx) { free(ctx); }
CGbool cgIsContext(CGcontext ctx) { return ctx ? CG_TRUE : CG_FALSE; }
const char *cgGetLastListing(CGcontext ctx) { return ctx ? ctx->listing : g_error_text; }
CGprogram cgCreateProgramFromFile(CGcontext ctx, CGenum type, const char *file, CGprofile profile, const char *entry, const char **args)
{
	(void)type; (void)file; (void)profile; (void)entry; (void)args;
	fail(ctx, CG_COMPILER_ERROR, "Cg compatibility shim has no Cg-to-GLSL compiler; Cg shader execution is unavailable");
	return NULL;
}
CGprogram cgCreateProgram(CGcontext ctx, CGenum type, const char *source, CGprofile profile, const char *entry, const char **args)
{
	(void)type; (void)source; (void)profile; (void)entry; (void)args;
	fail(ctx, CG_COMPILER_ERROR, "Cg compatibility shim has no Cg-to-GLSL compiler; Cg shader execution is unavailable");
	return NULL;
}
void cgDestroyProgram(CGprogram program) { free(program); }
CGbool cgIsProgram(CGprogram program) { return program ? CG_TRUE : CG_FALSE; }
void cgCompileProgram(CGprogram program) { if (program) program->compiled = 0; }
CGbool cgIsProgramCompiled(CGprogram program) { return program && program->compiled ? CG_TRUE : CG_FALSE; }
CGprofile cgGetProgramProfile(CGprogram program) { return program ? program->profile : CG_PROFILE_UNKNOWN; }
CGerror cgGetError(void) { CGerror error = g_error; g_error = CG_NO_ERROR; return error; }
const char *cgGetErrorString(CGerror error)
{
	switch (error) {
	case CG_NO_ERROR: return "No error";
	case CG_COMPILER_ERROR: return "Cg shader translation is unavailable";
	case CG_INVALID_PROFILE_ERROR: return "Unsupported Cg profile";
	case CG_INVALID_PROGRAM_ERROR: return "Invalid Cg program";
	case CG_INVALID_PARAMETER_ERROR: return "Invalid Cg parameter";
	default: return "Unknown Cg compatibility error";
	}
}
const char *cgGetLastErrorString(CGerror *error) { if (error) *error = g_error; return g_error_text; }
CGparameter cgGetNamedParameter(CGprogram program, const char *name)
{
	(void)program; (void)name;
	fail(NULL, CG_INVALID_PARAMETER_ERROR, "Cg parameters require a translated shader program");
	return NULL;
}
void cgDestroyParameter(CGparameter parameter) { free(parameter); }
void cgSetParameter1f(CGparameter parameter, float x) { if (parameter) parameter->value[0] = x; }
void cgSetParameter4fv(CGparameter parameter, const float *v) { if (parameter && v) memcpy(parameter->value, v, sizeof(parameter->value)); }

CGbool cgGLIsProfileSupported(CGprofile profile) { (void)profile; return CG_FALSE; }
void cgGLEnableProfile(CGprofile profile) { (void)profile; }
void cgGLDisableProfile(CGprofile profile) { (void)profile; }
CGprofile cgGLGetLatestProfile(CGenum type) { (void)type; return CG_PROFILE_UNKNOWN; }
void cgGLSetOptimalOptions(CGprofile profile) { (void)profile; }
void cgGLLoadProgram(CGprogram program) { (void)program; }
CGbool cgGLIsProgramLoaded(CGprogram program) { (void)program; return CG_FALSE; }
void cgGLBindProgram(CGprogram program) { (void)program; }
void cgGLUnbindProgram(CGprofile profile) { (void)profile; }
void cgGLSetParameter1f(CGparameter parameter, float x) { cgSetParameter1f(parameter, x); }
void cgGLSetParameter4fv(CGparameter parameter, const float *v) { cgSetParameter4fv(parameter, v); }
void cgGLSetStateMatrixParameter(CGparameter parameter, int matrix, int transform) { (void)parameter; (void)matrix; (void)transform; }
