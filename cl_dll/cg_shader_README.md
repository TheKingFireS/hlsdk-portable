# Cg-compatible translated shader pipeline status

This adapter recreates the names and lifecycle used by the MechMod `CCGShader` manager. It deliberately delegates compilation and binding to the Cg compatibility runtime, which currently supports only the explicitly translated shader subset.

This is not yet a full MechMod runtime port. The SDK's regular client does not call `CCGShader`; a mod branch must integrate the manager into its rendering hooks. Cg APIs are not NVIDIA ABI-compatible, and effects such as `samplerRECT` / `offsettex2D` need driver/context testing.
