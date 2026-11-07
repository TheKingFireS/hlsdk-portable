#include "cg_shader.h"
#include <stdio.h>
#include <string.h>

CCGShader g_CGShader;

Shader_s *CCGShader::FindShader(const char *name)
{
    for (Shader_s *shader = m_pHeadShader; shader; shader = shader->pNext)
        if (strcmp(shader->szName, name) == 0)
            return shader;
    return NULL;
}

CCGShader::CCGShader()
    : m_cgContext(NULL), m_cgVertProf(CG_PROFILE_UNKNOWN),
      m_cgFragProf(CG_PROFILE_UNKNOWN), m_pHeadShader(NULL),
      m_blIsSupported(false)
{
}

CCGShader::~CCGShader()
{
    RemoveAll();
    if (m_cgContext)
        cgDestroyContext(m_cgContext);
}

void CCGShader::SetShaderEnv(void)
{
    if (m_cgContext)
        cgDestroyContext(m_cgContext);
    m_cgContext = cgCreateContext();
    m_cgVertProf = cgGLGetLatestProfile(CG_GL_VERTEX);
    m_cgFragProf = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    m_blIsSupported = m_cgContext && m_cgVertProf != CG_PROFILE_UNKNOWN &&
        m_cgFragProf != CG_PROFILE_UNKNOWN;
}

bool CCGShader::IsSupported(void) { return m_blIsSupported; }
CGprofile CCGShader::GetFragProfile(void) { return m_cgFragProf; }
void CCGShader::StartVertProf(void) { if (m_blIsSupported) cgGLEnableProfile(m_cgVertProf); }
void CCGShader::StartFragProf(void) { if (m_blIsSupported) cgGLEnableProfile(m_cgFragProf); }
void CCGShader::EndVertProf(void) { cgGLDisableProfile(m_cgVertProf); }
void CCGShader::EndFragProf(void) { cgGLDisableProfile(m_cgFragProf); }

Shader_s *CCGShader::AddShader(char *name, int type)
{
    if (!name || !m_blIsSupported)
        return NULL;
    Shader_s *found = FindShader(name);
    if (found)
        return found;
    char path[512];
    const char *gameDir = gEngfuncs.pfnGetGameDirectory();
    if (!gameDir || snprintf(path, sizeof(path), "%s/shaders/%s", gameDir, name) >= (int)sizeof(path))
        return NULL;
    CGprofile profile = type == VERTEX_SHADER ? m_cgVertProf : m_cgFragProf;
    CGprogram program = cgCreateProgramFromFile(m_cgContext, CG_SOURCE, path, profile, "main", NULL);
    if (!program)
        return NULL;
    Shader_s *shader = new Shader_s;
    memset(shader, 0, sizeof(*shader));
    strncpy(shader->szName, name, sizeof(shader->szName) - 1);
    shader->iType = type;
    shader->cgProg = program;
    shader->pNext = m_pHeadShader;
    if (m_pHeadShader)
        m_pHeadShader->pPrev = shader;
    m_pHeadShader = shader;
    cgGLLoadProgram(program);
    return shader;
}

Shader_s *CCGShader::Remove(Shader_s *shader)
{
    if (!shader)
        return NULL;
    Shader_s *next = shader->pNext;
    if (shader->pPrev)
        shader->pPrev->pNext = shader->pNext;
    else
        m_pHeadShader = shader->pNext;
    if (shader->pNext)
        shader->pNext->pPrev = shader->pPrev;
    cgDestroyProgram(shader->cgProg);
    delete shader;
    return next;
}

void CCGShader::RemoveAll(void)
{
    while (m_pHeadShader)
        Remove(m_pHeadShader);
    m_blIsSupported = false;
}

void Shader_s::Bind(void)
{
    if (cgProg)
        cgGLBindProgram(cgProg);
}
