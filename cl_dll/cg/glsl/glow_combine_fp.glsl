#version 120
#extension GL_ARB_texture_rectangle : enable
varying vec2 vTex0;
varying vec2 vTex1;
uniform sampler2DRect texNormal;
uniform sampler2DRect texGlow;
void main() {
    gl_FragColor = texture2DRect(texNormal, vTex0) + 2.0 * texture2DRect(texGlow, vTex1);
}
