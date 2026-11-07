#version 120
#extension GL_ARB_texture_rectangle : enable
varying vec2 vTex0;
uniform sampler2DRect tex0;
void main() {
    vec4 c = texture2DRect(tex0, vTex0);
    gl_FragColor = c * c * c;
}
