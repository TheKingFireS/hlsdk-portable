#version 120
uniform sampler2D OffsetTex;
uniform sampler2D Identity;
uniform sampler2D DecalTex;
uniform sampler2D MaskTex;
uniform vec4 TexOffset;
varying vec2 vTex0;
varying vec2 vTex1;
varying vec2 vTex2;
varying vec2 vTex3;
void main() {
    vec4 flow = texture2D(OffsetTex, vTex0);
    vec4 lookup = texture2D(Identity, flow.gb);
    /* Cg offsettex2D used NVIDIA dependent texture reads. This approximation
       treats lookup.rg as a signed displacement around the identity midpoint. */
    vec2 uv = vTex2 + (lookup.rg - vec2(0.5)) * TexOffset.xy;
    vec4 color = texture2D(DecalTex, uv);
    color.a = 1.0;
    color *= texture2D(MaskTex, vTex3).r;
    gl_FragColor = color;
}
