#pragma once

/* Minimal client-side adapter for the MechMod Cg shader manager pattern.
 * Handles compile/bind/release for the known translated shader subset. */
#include "cg/cg.h"

#define FRAGMENT_SHADER 1
#define VERTEX_SHADER 2

struct Shader_s {
    char szName[64];
    int iType;
    CGprogram cgProg;
    Shader_s *pNext;
    Shader_s *pPrev;
    void Bind(void);
};

class CCGShader {
public:
    CCGShader();
    ~CCGShader();
    bool IsSupported(void);
    void RemoveAll(void);
    Shader_s *Remove(Shader_s *shader);
    Shader_s *AddShader(char *name, int type);
    void StartVertProf(void);
    void StartFragProf(void);
    void EndVertProf(void);
    void EndFragProf(void);
    CGprofile GetFragProfile(void);
    void SetShaderEnv(void);
private:
    Shader_s *FindShader(const char *name);
    CGcontext m_cgContext;
    CGprofile m_cgVertProf;
    CGprofile m_cgFragProf;
    Shader_s *m_pHeadShader;
    bool m_blIsSupported;
};

extern CCGShader g_CGShader;
