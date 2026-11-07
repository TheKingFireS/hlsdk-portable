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
    vec2 uv = vTex1 + TexOffset.xy * flow.xx + TexOffset.zw * flow.yy;
    vec4 color = texture2D(DecalTex, uv);
    color.a = 1.0;
    color *= texture2D(MaskTex, vTex3).r;
    gl_FragColor = color;
}
