#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME EarlyZ15


uniform int uDrawOpaque;
uniform vec2 uNearFar;

varying vec4 vViewVertex;



void main() {
// vars

vec4 tmp_0; float tmp_4 = 1.0;

// end vars

if(uDrawOpaque == 1 && tmp_4 < 1.0) discard;
float depth = (-vViewVertex.z * vViewVertex.w - uNearFar.x) / (uNearFar.y - uNearFar.x);
tmp_0 = fract(vec4(1.0, 255.0, 65025.0, 16581375.0) * depth);
tmp_0 -= tmp_0.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
tmp_0.a = uDrawOpaque == 1 ? 1.0 : tmp_4;
gl_FragColor = tmp_0.rgba;
}