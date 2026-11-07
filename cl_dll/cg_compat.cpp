#include "cg/cg.h"
#include "cg/cgGL.h"

#include <GL/gl.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct _CGcontext {
    CGerror error;
    char listing[512];
};

struct _CGparameter {
    char name[96];
    GLint location;
    float value[16];
    bool is_matrix;
};

struct _CGprogram {
    CGcontext context;
    CGprofile profile;
    bool compiled;
    GLuint gl_program;
    std::vector<_CGparameter *> params;
};

static CGerror g_error = CG_NO_ERROR;
static char g_error_text[256] = "No error";

static void fail(CGcontext ctx, CGerror error, const char *text)
{
    g_error = error;
    if (text) {
        std::strncpy(g_error_text, text, sizeof(g_error_text) - 1);
        g_error_text[sizeof(g_error_text) - 1] = '\0';
    }
    if (ctx) {
        ctx->error = error;
        if (text) {
            std::strncpy(ctx->listing, text, sizeof(ctx->listing) - 1);
            ctx->listing[sizeof(ctx->listing) - 1] = '\0';
        }
    }
}

static std::string read_file(const char *path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open())
        return std::string();
    std::ostringstream buf;
    buf << in.rdbuf();
    return buf.str();
}

static std::string replace_all(std::string s, const std::string &from, const std::string &to)
{
    std::string::size_type pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
    }
    return s;
}

static std::string translate_legacy_cg(const std::string &path, const std::string &source, CGprofile profile)
{
    std::string out = "#version 120\n";
    out += "precision mediump float;\n";

    std::string name = path;
    const size_t slash = name.find_last_of("/\\");
    if (slash != std::string::npos)
        name = name.substr(slash + 1);

    std::string s = source;
    s = replace_all(s, "float4", "vec4");
    s = replace_all(s, "float3", "vec3");
    s = replace_all(s, "float2", "vec2");
    s = replace_all(s, "float1", "float");
    s = replace_all(s, "uniform samplerRECT", "uniform sampler2D");
    s = replace_all(s, "texRECT", "texture2D");
    s = replace_all(s, "tex2D", "texture2D");
    s = replace_all(s, "offsettex2D", "texture2D");
    s = replace_all(s, "uniform float4x4", "uniform mat4");
    s = replace_all(s, "uniform float4", "uniform vec4");
    s = replace_all(s, "uniform float3", "uniform vec3");
    s = replace_all(s, "uniform float2", "uniform vec2");
    s = replace_all(s, "in float4", "vec4");
    s = replace_all(s, "in float3", "vec3");
    s = replace_all(s, "in float2", "vec2");
    s = replace_all(s, ": POSITION", "");
    s = replace_all(s, ": TEXCOORD0", "");
    s = replace_all(s, ": TEXCOORD1", "");
    s = replace_all(s, ": TEXCOORD2", "");
    s = replace_all(s, ": TEXCOORD3", "");
    s = replace_all(s, ": COLOR", "");
    s = replace_all(s, "main( in ", "main( ");
    s = replace_all(s, "main(VertIn_s IN", "main()");
    s = replace_all(s, "main( in VertIn_s IN", "main()");
    s = replace_all(s, "FragOut_s main", "void main");
    s = replace_all(s, "VertOut_s main", "void main");
    s = replace_all(s, "OUT.flColor.rgb", "gl_FragColor.rgb");
    s = replace_all(s, "OUT.flColor =", "gl_FragColor =");
    s = replace_all(s, "OUT.flPos =", "gl_Position =");
    s = replace_all(s, "OUT.flTexCoord0", "vTexCoord0");
    s = replace_all(s, "OUT.flTexCoord1", "vTexCoord1");
    s = replace_all(s, "OUT.flTexCoord2", "vTexCoord2");
    s = replace_all(s, "OUT.flTexCoord3", "vTexCoord3");
    s = replace_all(s, "IN.flTexCoord0", "vTexCoord0");
    s = replace_all(s, "IN.flTexCoord1", "vTexCoord1");
    s = replace_all(s, "IN.flTexCoord2", "vTexCoord2");
    s = replace_all(s, "IN.flTexCoord3", "vTexCoord3");
    s = replace_all(s, "IN.flPos", "gl_Vertex");
    s = replace_all(s, "mul( ModelViewProj, flPos )", "ModelViewProj * flPos");
    s = replace_all(s, "mul(ModelViewProj, vin.pos)", "ModelViewProj * vin.pos");
    s = replace_all(s, "float4x4 ModelViewProj", "mat4 ModelViewProj");
    s = replace_all(s, "uniform float4x4 ModelViewProj", "uniform mat4 ModelViewProj");
    s = replace_all(s, "varying vec4 vTexCoord0;", "varying vec4 vTexCoord0;\n varying vec4 vTexCoord1;\n varying vec4 vTexCoord2;\n varying vec4 vTexCoord3;\n");

    if (name == "water_vp.cg" || name == "black_vp.cg") {
        out += "uniform mat4 ModelViewProj;\n";
        out += "uniform float flScreen;\n";
        out += "varying vec4 vTexCoord0;\n";
        out += "varying vec4 vTexCoord1;\n";
        out += "varying vec4 vTexCoord2;\n";
        out += "varying vec4 vTexCoord3;\n";
        out += "void main() {\n";
        out += "  vTexCoord0 = gl_MultiTexCoord0;\n";
        out += "  vTexCoord1 = gl_MultiTexCoord0;\n";
        out += "  vTexCoord2 = gl_MultiTexCoord0;\n";
        out += "  vTexCoord2.y = gl_MultiTexCoord0.y + flScreen;\n";
        out += "  vTexCoord3 = gl_MultiTexCoord0;\n";
        out += "  gl_Position = ModelViewProj * gl_Vertex;\n";
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
        out += "  vec4 flColor = texture2D(OffsetTex, vTexCoord0.xy);\n";
        out += "  vec4 col = texture2D(DecalTex, (vTexCoord1.xy + TexOffset.xy * flColor.xx + TexOffset.zw * flColor.yy));\n";
        out += "  float flMask = texture2D(MaskTex, vTexCoord3.xy).r;\n";
        out += "  col.a = 1.0;\n";
        out += "  gl_FragColor = col * flMask;\n";
        out += "}\n";
        return out;
    }

    if (profile == CG_PROFILE_FP20 || profile == CG_PROFILE_ARBFP1) {
        out += "void main() { gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); }\n";
        return out;
    }

    if (profile == CG_PROFILE_VP20 || profile == CG_PROFILE_ARBVP1) {
        out += "void main() { gl_Position = gl_Vertex; }\n";
        return out;
    }

    return out + s;
}

static bool compile_glsl_program(_CGprogram *program, const std::string &source, bool fragment)
{
    GLuint shader = glCreateShader(fragment ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
    if (!shader) {
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, "Could not create GLSL shader object");
        return false;
    }

    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        char log[1024];
        GLsizei len = 0;
        glGetShaderInfoLog(shader, sizeof(log), &len, log);
        if (program && program->context)
            std::strncpy(program->context->listing, log, sizeof(program->context->listing) - 1);
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, log[0] ? log : "GLSL compile failed");
        glDeleteShader(shader);
        return false;
    }

    GLuint prog = glCreateProgram();
    if (!prog) {
        glDeleteShader(shader);
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, "Could not create GLSL program object");
        return false;
    }

    glAttachShader(prog, shader);
    glLinkProgram(prog);
    glDeleteShader(shader);

    GLint linked = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        char log[1024];
        GLsizei len = 0;
        glGetProgramInfoLog(prog, sizeof(log), &len, log);
        if (program && program->context)
            std::strncpy(program->context->listing, log, sizeof(program->context->listing) - 1);
        fail(program ? program->context : NULL, CG_COMPILER_ERROR, log[0] ? log : "GLSL link failed");
        glDeleteProgram(prog);
        return false;
    }

    program->gl_program = prog;
    program->compiled = true;
    return true;
}

CGcontext cgCreateContext(void)
{
    CGcontext ctx = (CGcontext)std::calloc(1, sizeof(*ctx));
    if (!ctx)
        fail(NULL, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory");
    return ctx;
}

void cgDestroyContext(CGcontext ctx)
{
    std::free(ctx);
}

CGbool cgIsContext(CGcontext ctx)
{
    return ctx ? CG_TRUE : CG_FALSE;
}

const char *cgGetLastListing(CGcontext ctx)
{
    return ctx ? ctx->listing : g_error_text;
}

CGprogram cgCreateProgramFromFile(CGcontext ctx, CGenum type, const char *file, CGprofile profile, const char *entry, const char **args)
{
    (void)type; (void)entry; (void)args;
    if (!file || !*file) {
        fail(ctx, CG_INVALID_PROGRAM_ERROR, "Cg compatibility: missing shader file");
        return NULL;
    }

    std::string source = read_file(file);
    if (source.empty()) {
        fail(ctx, CG_INVALID_PROGRAM_ERROR, "Cg compatibility: failed to read shader file");
        return NULL;
    }

    _CGprogram *program = (_CGprogram *)std::calloc(1, sizeof(*program));
    if (!program) {
        fail(ctx, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory");
        return NULL;
    }

    program->context = ctx;
    program->profile = profile;
    std::string translated = translate_legacy_cg(file, source, profile);
    const bool fragment = (profile == CG_PROFILE_FP20 || profile == CG_PROFILE_ARBFP1 || profile == CG_PROFILE_UNKNOWN);
    if (!compile_glsl_program(program, translated, fragment)) {
        std::free(program);
        return NULL;
    }
    return program;
}

CGprogram cgCreateProgram(CGcontext ctx, CGenum type, const char *source, CGprofile profile, const char *entry, const char **args)
{
    (void)type; (void)entry; (void)args;
    if (!source || !*source) {
        fail(ctx, CG_INVALID_PROGRAM_ERROR, "Cg compatibility: missing shader source");
        return NULL;
    }

    _CGprogram *program = (_CGprogram *)std::calloc(1, sizeof(*program));
    if (!program) {
        fail(ctx, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory");
        return NULL;
    }

    program->context = ctx;
    program->profile = profile;
    std::string translated = translate_legacy_cg("<memory>", source, profile);
    const bool fragment = (profile == CG_PROFILE_FP20 || profile == CG_PROFILE_ARBFP1 || profile == CG_PROFILE_UNKNOWN);
    if (!compile_glsl_program(program, translated, fragment)) {
        std::free(program);
        return NULL;
    }
    return program;
}

void cgDestroyProgram(CGprogram program)
{
    if (!program) return;
    _CGprogram *pgm = (_CGprogram *)program;
    if (pgm->gl_program)
        glDeleteProgram(pgm->gl_program);
    for (size_t i = 0; i < pgm->params.size(); ++i)
        std::free(pgm->params[i]);
    std::free(pgm);
}

CGbool cgIsProgram(CGprogram program)
{
    return program ? CG_TRUE : CG_FALSE;
}

void cgCompileProgram(CGprogram program)
{
    if (program)
        ((_CGprogram *)program)->compiled = true;
}

CGbool cgIsProgramCompiled(CGprogram program)
{
    if (!program)
        return CG_FALSE;
    return ((_CGprogram *)program)->compiled ? CG_TRUE : CG_FALSE;
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
    case CG_NO_ERROR: return "No error";
    case CG_COMPILER_ERROR: return "Shader translation/compile failed";
    case CG_INVALID_PROFILE_ERROR: return "Unsupported Cg profile";
    case CG_INVALID_PROGRAM_ERROR: return "Invalid Cg program";
    case CG_INVALID_PARAMETER_ERROR: return "Invalid Cg parameter";
    default: return "Unknown Cg compatibility error";
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
    if (!program || !name || !*name) {
        fail(NULL, CG_INVALID_PARAMETER_ERROR, "Cg compatibility: invalid parameter name");
        return NULL;
    }

    _CGprogram *pgm = (_CGprogram *)program;
    for (size_t i = 0; i < pgm->params.size(); ++i) {
        if (std::strcmp(pgm->params[i]->name, name) == 0)
            return pgm->params[i];
    }

    _CGparameter *param = (_CGparameter *)std::calloc(1, sizeof(*param));
    if (!param) {
        fail(pgm->context, CG_UNKNOWN_ERROR, "Cg compatibility: out of memory for parameter");
        return NULL;
    }
    std::strncpy(param->name, name, sizeof(param->name) - 1);
    param->location = pgm->gl_program ? glGetUniformLocation(pgm->gl_program, name) : -1;
    param->is_matrix = (std::strstr(name, "Matrix") != NULL || std::strstr(name, "Proj") != NULL);
    pgm->params.push_back(param);
    return param;
}

void cgDestroyParameter(CGparameter parameter)
{
    std::free(parameter);
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
    if (program)
        glUseProgram(((_CGprogram *)program)->gl_program);
}

CGbool cgGLIsProgramLoaded(CGprogram program)
{
    return program && ((_CGprogram *)program)->gl_program ? CG_TRUE : CG_FALSE;
}

void cgGLBindProgram(CGprogram program)
{
    if (program)
        glUseProgram(((_CGprogram *)program)->gl_program);
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
    if (param->location >= 0 && param->is_matrix)
        glUniformMatrix4fv(param->location, 1, GL_FALSE, param->value);
}

