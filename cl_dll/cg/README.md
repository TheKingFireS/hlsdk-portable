#pragma once

/* The shim allows legacy client code to compile and links its symbols directly
 * into client.dll. It is not a shader compiler: Cg programs fail cleanly with
 * CG_COMPILER_ERROR until the Cg-to-GLSL/nvFX translator is implemented. */
