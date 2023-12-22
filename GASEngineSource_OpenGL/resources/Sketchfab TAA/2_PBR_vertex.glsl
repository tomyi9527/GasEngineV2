#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME PBR(material_0)


attribute vec3 Vertex;
attribute vec3 Normal;

uniform float uPointSize;
uniform mat3 uModelNormalMatrix;
uniform mat3 uModelViewNormalMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec2 RenderSize;
uniform vec4 uHalton;

varying vec3 vViewNormal;
varying vec3 vModelVertex;
varying vec3 vModelNormal;
varying vec4 vViewVertex;



void main() {
// vars

vec4 viewVertex; mat4 jitteredProjection; vec3 modelVertex; vec3 modelNormal; vec3 viewNormal;

// end vars

viewVertex = uModelViewMatrix*vec4(Vertex.xyz, 1.);
gl_PointSize = min(64.0, max(1.0, -uPointSize / viewVertex.z));


 vec4 ndcPos = uProjectionMatrix * viewVertex;
 ndcPos.xyz /= ndcPos.w;
 float fact = 1.0;
 if( ndcPos.x > 0.0 && abs( uHalton.z ) == 1.0 )
   fact = 0.0;
 jitteredProjection = uProjectionMatrix;
 float doPerp = jitteredProjection[3][3] == 0.0 ? 1.0 : 0.0;
 float doOrtho = doPerp == 1.0 ? 0.0 : 1.0;
 vec2 halt = fact * ( uHalton.xy / RenderSize.xy );
 // persp
 jitteredProjection[2][0] += doPerp * halt.x;
 jitteredProjection[2][1] += doPerp * halt.y;
 // ortho
 jitteredProjection[3][0] += doOrtho * halt.x;
 jitteredProjection[3][1] += doOrtho * halt.y;

gl_Position = jitteredProjection*viewVertex;
viewNormal = uModelViewNormalMatrix*Normal.xyz;
vViewNormal = viewNormal.rgb;
vViewVertex = viewVertex.rgba;
modelNormal = uModelNormalMatrix*Normal.xyz;
vModelNormal = modelNormal.rgb;
modelVertex = vec3(uModelMatrix*vec4(Vertex.xyz, 1.));
vModelVertex = modelVertex.rgb;
}