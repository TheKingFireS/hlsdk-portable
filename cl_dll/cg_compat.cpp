#include "cg/cg.h"
#include "cg/cgGL.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct _CGcontext {
    CGerror error;
    char listing[256];
};

struct _CGparameter {
    char name[96];
    GLint location;
    float value[16];
    int isMatrix;
};

struct _CGprogram {
    CGcontext context;
    CGprofile profile;
    int compiled;
    GLuint glProgram;
    GLuint glShader;
    std::vector<_CGparameter *> params;
    std::string source;
};

static CGerror g_error = CG_NO_ERROR;
static char g_error_text[256] = "No error";

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

static std::string ReadTextFile(const char *path)
{
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in.is_open())
        return std::string();
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

static std::string StripPath(const std::string &path)
{
    size_t slash = path.find_last_of("/\\");
    if (slash == std::string::npos)
        return path;
    return path.substr(slash + 1);
}

static std::string TranslateBuiltInShader(const std::string &shaderPath, const std::string &source,
                                          bool fragment)
{
    std::string name = StripPath(shaderPath);
    const char *txt = source.c_str();
    (void)txt;

    std::string out = "#version 120\n";
    out += "precision mediump float;\n";

    if (name == "water_vp.cg" || name == "black_vp.cg") {
        out += "uniform mat4 ModelViewProj;\n";
        out += "uniform float flScreen;\n";
        out += "varying vec4 vTexCoord0;\n";
        out += "varying vec4 vTexCoord1;\n";
        out += "varying vec4 vTexCoord2;\n";
        out += "varying vec4 vTexCoord3;\n";
        out += "void main() {\n";
        out += "    vec4 flPos = gl_Vertex;\n";
        out += "    vec4 flTexCoord0 = gl_MultiTexCoord0;\n";
        out += "    vTexCoord0 = flTexCoord0;\n";
        out += "    vTexCoord1 = flTexCoord0;\n";
        out += "    vTexCoord2 = flTexCoord0;\n";
        out += "    vTexCoord2.y = flTexCoord0.y + flScreen;\n";
        out += "    vTexCoord3 = flTexCoord0;\n";
        out += "    gl_Position = ModelViewProj * flPos;\n";
        out += "}\n";
        return out;
    }

    if (name == "water_fp.cg" || name == "black_fp.cg" || name == "fp20_water_fp.cg" || name == "fp20_black_fp.cg") {
        out += "uniform sampler2D OffsetTex;\n";
        out += "uniform sampler2D Identity;\n";
        out += "uniform sampler2D DecalTex;\n";
        out += "uniform sampler2D MaskTex;\n";
        out += "uniform vec4 TexOffset;\n";
        out += "varying vec4 vTexCoord0;\n";
        out += "varying vec4 vTexCoord1;\n";
        out += "varying vec4 vTexCoord2;\n";
        out += "varying vec4 vTexCoord3;\n";
        out += "void main() {\n";
        out += "    vec4 flColor = texture2D(OffsetTex, vTexCoord0.xy);\n";
        if (name == "water_fp.cg") {
            out += "    vec2 flTex = vTexCoord1.xy + TexOffset.xy * flColor.xx + TexOffset.zw * flColor.yy;\n";
            out += "    vec4 col = texture2D(DecalTex, flTex.xy);\n";
            out += "    float flMask = texture2D(MaskTex, vTexCoord3.xy).r;\n";
            out += "    col.a = 1.0;\n";
            out += "    gl_FragColor = col * flMask;\n";
        } else {
            out += "    vec4 flPrevLookup = texture2D(Identity, flColor.gb);\n";
            out += "    vec4 col = texture2D(DecalTex, vTexCoord2.xy + (flPrevLookup.xy * TexOffset.xy));\n";
            out += "    float flMask = texture2D(MaskTex, vTexCoord3.xy).r;\n";
            out += "    col.a = 1.0;\n";
            out += "    gl_FragColor = col * flMask;\n";
        }
        out += "}\n";
        return out;
    }

    if (fragment) {
        out += "void main() { gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); }\n";
        return out;
    }

    out += "void main() { gl_Position = vec4(0.0); }\n";
    return out;
}

static std::string TranslateCgSource(const std::string &shaderPath, const std::string &source, CGprofile profile)
{
    const bool fragment = (profile == CG_PROFILE_FP20 || profile == CG_PROFILE_ARBFP1 || profile == CG_PROFILE_UNKNOWN);
    std::string known = TranslateBuiltInShader(shaderPath, source, fragment);
    if (!known.empty())
        return known;

    std::string translated = source;
    std::string f1 = "float4"; std::string r1 = "vec4";
    std::string f2 = "float3"; std::string r2 = "vec3";
    std::string f3 = "float2"; std::string r3 = "vec2";
    std::string f4 = "float";  std::string r4 = "float";
    std::string::size_type pos = 0;
    while ((pos = translated.find(f1, pos)) != std::string::npos) {
        translated.replace(pos, f1.size(), r1); pos += r1.size();
    }
    while ((pos = translated.find(f2, pos)) != std::string::npos) {
        translated.replace(pos, f2.size(), r2); pos += r2.size();
    }
    while ((pos = translated.find(f3, pos)) != std::string::npos) {
        translated.replace(pos, f3.size(), r3); pos += r3.size();
    }
    while ((pos = translated.find(f4, pos)) != std::string::npos) {
        translated.replace(pos, f4.size(), r4); pos += r4.size();
    }
    std::string replacements[] = {
        ": POSITION", "",
        ": TEXCOORD0", "",
        ": TEXCOORD1", "",
        ": TEXCOORD2", "",
        ": TEXCOORD3", "",
        ": COLOR", "",
        "uniform samplerRECT", "uniform sampler2D",
        "texRECT", "texture2D",
        "tex2D", "texture2D",
        "offsettex2D", "texture2D",
        "in float4", "vec4",
        "in float3", "vec3",
        "in float2", "vec2",
        "in uniform", "uniform",
        "uniform float4x4", "uniform mat4",
        "uniform float4", "uniform vec4",
        "uniform float3", "uniform vec3",
        "uniform float2", "uniform vec2",
        "struct VertOut_s {", "varying vec4 vTexCoord0;\n varying vec4 vTexCoord1;\n varying vec4 vTexCoord2;\n varying vec4 vTexCoord3;\n struct VertOut_s {",
        "struct FragOut_s {", "struct FragOut_s {"
    };
    for (size_t i = 0; i < sizeof(replacements) / sizeof(replacements[0]); i += 2) {
        std::string::size_type p = 0;
        while ((p = translated.find(replacements[i], p)) != std::string::npos)
            translated.replace(p, replacements[i].size(), replacements[i + 1]);
    }
    return translated;
}

static bool CompileGLSLProgram(_CGprogram *program, const std::string &source, bool fragment)
{
    const char *str = source.c_str();
    GLuint shader = glCreateShader(fragment ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
    if (!shader) {
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, "Cg compatibility: could not create GLSL shader object");
        return false;
    }

    glShaderSource(shader, 1, &str, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        char log[1024] = {0};
        GLsizei len = 0;
        glGetShaderInfoLog(shader, sizeof(log), &len, log);
        if (program && program->context)
            strncpy(program->context->listing, log, sizeof(program->context->listing) - 1);
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, log[0] ? log : "Cg compatibility: GLSL compile failed");
        glDeleteShader(shader);
        return false;
    }

    GLuint glProgram = glCreateProgram();
    if (!glProgram) {
        glDeleteShader(shader);
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, "Cg compatibility: could not create GLSL program object");
        return false;
    }

    glAttachShader(glProgram, shader);
    glLinkProgram(glProgram);
    glDeleteShader(shader);

    glGetProgramiv(glProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        char log[1024] = {0};
        GLsizei len = 0;
        glGetProgramInfoLog(glProgram, sizeof(log), &len, log);
        if (program && program->context)
            strncpy(program->context->listing, log, sizeof(program->context->listing) - 1);
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, log[0] ? log : "Cg compatibility: GLSL link failed");
        glDeleteProgram(glProgram);
        return false;
    }

    program->glShader = shader;
    program->glProgram = glProgram;
    program->compiled = 1;
    return true;
}

CGcontext cgCreateContext(void)
{
    CGcontext ctx = (CGcontext)calloc(1, sizeof(*ctx));
    if (!ctx) {
        fail(NULL, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory");
        return NULL;
    }
    ctx->error = CG_NO_ERROR;
    snprintf(ctx->listing, sizeof(ctx->listing), "No error");
    return ctx;
}

void cgDestroyContext(CGcontext ctx)
{
    if (!ctx) return;
    free(ctx);
}

CGbool cgIsContext(CGcontext ctx)
{
    return ctx ? CG_TRUE : CG_FALSE;
}

const char *cgGetLastListing(CGcontext ctx)
{
    return ctx ? ctx->listing : g_error_text;
}

CGprogram cgCreateProgramFromFile(CGcontext ctx, CGenum program_type, const char *program_file, CGprofile profile, const char *entry, const char **args)
{
    (void)program_type; (void)entry; (void)args;
    if (!program_file || !*program_file) {
        fail(ctx, CG_INVALID_PROGRAM_ERROR, "Cg compatibility: missing shader file");
        return NULL;
    }

    std::string file = ReadTextFile(program_file);
    if (file.empty()) {
        fail(ctx, CG_INVALID_PROGRAM_ERROR, "Cg compatibility: failed to read shader file");
        return NULL;
    }

    _CGprogram *program = (_CGprogram *)calloc(1, sizeof(*program));
    if (!program) {
        fail(ctx, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory");
        return NULL;
    }
    program->context = ctx;
    program->profile = profile;
    program->source = TranslateCgSource(program_file, file, profile);
    const bool fragment = (profile == CG_PROFILE_FP20 || profile == CG_PROFILE_ARBFP1 || profile == CG_PROFILE_UNKNOWN);
    if (!CompileGLSLProgram(program, program->source, fragment)) {
        free(program);
        return NULL;
    }
    return program;
}

CGprogram cgCreateProgram(CGcontext ctx, CGenum program_type, const char *program_source, CGprofile profile, const char *entry, const char **args)
{
    (void)program_type; (void)entry; (void)args;
    if (!program_source || !*program_source) {
        fail(ctx, CG_INVALID_PROGRAM_ERROR, "Cg compatibility: missing shader source");
        return NULL;
    }

    _CGprogram *program = (_CGprogram *)calloc(1, sizeof(*program));
    if (!program) {
        fail(ctx, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory");
        return NULL;
    }
    program->context = ctx;
    program->profile = profile;
    program->source = TranslateCgSource("<memory>", program_source, profile);
    const bool fragment = (profile == CG_PROFILE_FP20 || profile == CG_PROFILE_ARBFP1 || profile == CG_PROFILE_UNKNOWN);
    if (!CompileGLSLProgram(program, program->source, fragment)) {
        free(program);
        return NULL;
    }
    return program;
}

void cgDestroyProgram(CGprogram program)
{
    if (!program) return;
    if (((_CGprogram *)program)->glProgram)
        glDeleteProgram(((_CGprogram *)program)->glProgram);
    for (size_t i = 0; i < ((const _CGprogram *)program)->params.size(); ++i)
        free(((const _CGprogram *)program)->params[i]);
    free(program);
}

CGbool cgIsProgram(CGprogram program)
{
    return program ? CG_TRUE : CG_FALSE;
}

void cgCompileProgram(CGprogram program)
{
    if (!program) return;
    ((_CGprogram *)program)->compiled = 1;
}

CGbool cgIsProgramCompiled(CGprogram program)
{
    return program && ((const _CGprogram *)program)->compiled ? CG_TRUE : CG_FALSE;
}

CGprofile cgGetProgramProfile(CGprogram program)
{
    return program ? ((_CGprogram *)program)->profile : CG_PROFILE_UNKNOWN;
}

CGerror cgGetError(void)
{
    CGerror e = g_error;
    g_error = CG_NO_ERROR;
    return e;
}

const char *cgGetErrorString(CGerror error)
{
    switch (error) {
    case CG_NO_ERROR:
        return "No error";
    case CG_COMPILER_ERROR:
        return "Cg shader translation or compile failed";
    case CG_INVALID_PROFILE_ERROR:
        return "Unsupported Cg profile";
    case CG_INVALID_PROGRAM_ERROR:
        return "Invalid Cg program";
    case CG_INVALID_PARAMETER_ERROR:
        return "Invalid Cg parameter";
    default:
        return "Unknown Cg compatibility error";
    }
}

const char *cgGetLastErrorString(CGerror *error)
{
    if (error)
        *error = g_error;
    return g_error_text;
}

CGparameter cgGetNamedParameter(CGprogram program, const char *name)
{
    if (!program || !name || !name[0]) {
        fail(NULL, CG_INVALID_PARAMETER_ERROR, "Cg compatibility: parameter lookup requires a valid name");
        return NULL;
    }

    _CGprogram *pgm = (_CGprogram *)program;
    for (size_t i = 0; i < pgm->params.size(); ++i) {
        if (strcmp(pgm->params[i]->name, name) == 0)
            return pgm->params[i];
    }

    _CGparameter *param = (_CGparameter *)calloc(1, sizeof(*param));
    if (!param) {
        fail(pgm->context, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory creating parameter");
        return NULL;
    }
    strncpy(param->name, name, sizeof(param->name) - 1);
    param->location = pgm->glProgram ? glGetUniformLocation(pgm->glProgram, name) : -1;
    param->isMatrix = (strstr(name, "Matrix") != NULL || strstr(name, "Proj") != NULL);
    pgm->params.push_back(param);
    return param;
}

void cgDestroyParameter(CGparameter parameter)
{
    free(parameter);
}

void cgSetParameter1f(CGparameter parameter, float x)
{
    if (!parameter)
        return;
    _CGparameter *param = (_CGparameter *)parameter;
    param->value[0] = x;
    if (param->location >= 0)
        glUniform1f(param->location, x);
}

void cgSetParameter4f(CGparameter parameter, float x, float y, float z, float w)
{
    if (!parameter)
        return;
    _CGparameter *param = (_CGparameter *)parameter;
    param->value[0] = x; param->value[1] = y; param->value[2] = z; param->value[3] = w;
    if (param->location >= 0)
        glUniform4f(param->location, x, y, z, w);
}

void cgSetParameter4fv(CGparameter parameter, const float *values)
{
    if (!parameter || !values)
        return;
    _CGparameter *param = (_CGparameter *)parameter;
    for (int i = 0; i < 4; ++i)
        param->value[i] = values[i];
    if (param->location >= 0)
        glUniform4fv(param->location, 1, values);
}

CGbool cgGLIsProfileSupported(CGprofile profile)
{
    return (profile == CG_PROFILE_VP20 || profile == CG_PROFILE_FP20 || profile == CG_PROFILE_ARBVP1 || profile == CG_PROFILE_ARBFP1) ? CG_TRUE : CG_FALSE;
}

void cgGLEnableProfile(CGprofile profile)
{
    (void)profile;
}

void cgGLDisableProfile(CGprofile profile)
{
    (void)profile;
}

CGprofile cgGLGetLatestProfile(CGenum profile_type)
{
    if (profile_type == CG_GL_VERTEX)
        return CG_PROFILE_VP20;
    if (profile_type == CG_GL_FRAGMENT)
        return CG_PROFILE_FP20;
    return CG_PROFILE_UNKNOWN;
}

void cgGLSetOptimalOptions(CGprofile profile)
{
    (void)profile;
}

void cgGLLoadProgram(CGprogram program)
{
    if (!program)
        return;
    glUseProgram(((_CGprogram *)program)->glProgram);
}

CGbool cgGLIsProgramLoaded(CGprogram program)
{
    return program && ((_CGprogram *)program)->glProgram ? CG_TRUE : CG_FALSE;
}

void cgGLBindProgram(CGprogram program)
{
    if (program)
        glUseProgram(((_CGprogram *)program)->glProgram);
}

void cgGLUnbindProgram(CGprofile profile)
{
    (void)profile;
    glUseProgram(0);
}

void cgGLSetParameter1f(CGparameter parameter, float x)
{
    cgSetParameter1f(parameter, x);
}

void cgGLSetParameter4f(CGparameter parameter, float x, float y, float z, float w)
{
    cgSetParameter4f(parameter, x, y, z, w);
}

void cgGLSetParameter4fv(CGparameter parameter, const float *values)
{
    cgSetParameter4fv(parameter, values);
}

void cgGLSetStateMatrixParameter(CGparameter parameter, int matrix, int transform)
{
    (void)matrix; (void)transform;
    if (!parameter)
        return;
    _CGparameter *param = (_CGparameter *)parameter;
    if (param->location >= 0 && param->isMatrix) {
        glUniformMatrix4fv(param->location, 1, GL_FALSE, param->value);
    }
}

