# Legacy Cg shader translation notes

These GLSL 1.20 files are hand translations of the Cg sources shared for MechMod. They use the compatibility-profile built-ins (`gl_Vertex`, `gl_MultiTexCoord*`, and `gl_ModelViewProjectionMatrix`) to fit the original immediate-mode GoldSource rendering calls.

This is a translation reference, not yet a runtime replacement: `cg_compat.cpp` still has no shader compiler, no OpenGL program loader, and no nvFX container/program dispatch. Sampler uniforms must be assigned texture units by the renderer. `samplerRECT` translations require `GL_ARB_texture_rectangle` and rectangle texture coordinates in pixels. `fp20_water_fp.glsl` is approximate because NVIDIA's `offsettex2D` intrinsic semantics are profile-specific; `fp20_black_fp.glsl` also approximates the FP20 `expand` intrinsic. Validate those effects against the original Cg runtime before treating output as equivalent.
