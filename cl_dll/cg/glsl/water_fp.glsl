#version 120
precision mediump float;

uniform sampler2D OffsetTex;
uniform sampler2D Identity;
uniform sampler2D DecalTex;
uniform sampler2D MaskTex;
uniform vec4 TexOffset;

varying vec4 vTexCoord0;
varying vec4 vTexCoord1;
varying vec4 vTexCoord2;
varying vec4 vTexCoord3;

void main()
{
    vec4 flColor = texture2D(OffsetTex, vTexCoord0.xy);
    vec2 flTex = vTexCoord1.xy + TexOffset.xy * flColor.xx + TexOffset.zw * flColor.yy;
    vec4 col = texture2D(DecalTex, flTex.xy);
    float flMask = texture2D(MaskTex, vTexCoord3.xy).r;
    col.a = 1.0;
    gl_FragColor = col * flMask;
}
