#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME PBR15


attribute vec3 Vertex;
attribute vec2 TexCoord0;
attribute vec3 Normal;

uniform mat4 uModelViewMatrix;
uniform mat4 uModelViewNormalMatrix;
uniform mat4 uProjectionMatrix;
uniform vec2 RenderSize;
uniform vec4 uHalton;

varying vec3 vViewNormal;
varying vec2 vTexCoord0;
varying vec4 vViewVertex;



void main() {
// vars

const float floatWhite = float(1.0); mat4 jitteredProjection; vec4 viewVertex; vec3 viewNormal;

// end vars

gl_PointSize = floatWhite;

 jitteredProjection = uProjectionMatrix;
 float doPerp = jitteredProjection[3][3] == 0.0 ? 1.0 : 0.0;
 float doOrtho = doPerp == 1.0 ? 0.0 : 1.0;
 vec2 halt = (abs(uHalton.z) == 2.0 ? 1.0 : 0.0) * (uHalton.xy / RenderSize.xy);
 // persp
 jitteredProjection[2][0] += doPerp * halt.x;
 jitteredProjection[2][1] += doPerp * halt.y;
 // ortho
 jitteredProjection[3][0] += doOrtho * halt.x;
 jitteredProjection[3][1] += doOrtho * halt.y;

viewVertex = uModelViewMatrix*vec4(Vertex.xyz, 1.);
gl_Position = jitteredProjection*viewVertex;
viewNormal = vec3(uModelViewNormalMatrix*vec4(Normal.xyz, 0.));
vViewNormal = viewNormal.rgb;
vTexCoord0 = TexCoord0.rg;
vViewVertex = viewVertex.rgba;
}