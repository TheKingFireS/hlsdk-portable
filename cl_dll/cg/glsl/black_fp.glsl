#version 120
precision mediump float;

uniform sampler2D DecalTex;

varying vec2 vTex0;

void main()
{
    vec4 color = texture2D(DecalTex, vTex0);
    float lum = (color.r + color.g + color.b) * 0.5;
    lum = 2.0 * (lum - 0.5);
    gl_FragColor.rgb = 0.2 - lum;
    gl_FragColor.a = 1.0;
}
