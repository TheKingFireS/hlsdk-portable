# Legacy Cg 1.2 compatibility layer

This directory provides a small compatibility shim for classic MechMod / Cg 1.2 expectations so older shader-management code can still compile against the portable SDK without pulling in NVIDIA's proprietary Cg runtime.

The layer intentionally does not replace a real Cg compiler or runtime. It provides the old symbols used by older Half-Life mods and lets the code build cleanly while decoding incompatibilities in a more portable shader path.

What is included:
- `cg.h` — Cg 1.2 API surface used by MechMod-era code.
- `cgGL.h` — OpenGL integration surface for profile selection and parameter binding.
- `cg_compat.cpp` — compatibility runtime with stub implementations and parameter tracking.

Notes:
- This is a compatibility bridge, not a complete Cg implementation.
- Actual shader translation still requires nvFX/GLSL or an original Cg compiler pipeline.
- For builds using this shim, enable the CMake option `HLSDK_ENABLE_CG_COMPAT=ON`.
