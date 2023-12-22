#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME EarlyZ(material_0)


uniform int uDrawOpaque;
uniform vec2 uNearFar;

varying vec4 vViewVertex;



void main() {
// vars

vec4 tmp_0;

// end vars

float depth = (-vViewVertex.z * vViewVertex.w - uNearFar.x) / (uNearFar.y - uNearFar.x);
tmp_0.rgb = vec3(1.0, 255.0, 65025.0) * depth;
tmp_0.gb = fract(tmp_0.gb);
tmp_0.rg -= tmp_0.gb * (1.0/256.0);
tmp_0.a = 1.0;
gl_FragColor = tmp_0.rgba;
}