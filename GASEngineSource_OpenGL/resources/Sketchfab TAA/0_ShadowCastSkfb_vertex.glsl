#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME ShadowCastSkfb


attribute vec3 Vertex;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec2 RenderSize;
uniform vec4 uHalton;

varying vec4 vViewVertex;



void main() {
// vars

const float floatWhite = float(1.0); mat4 jitteredProjection; vec4 viewVertex;

// end vars

gl_PointSize = floatWhite;
viewVertex = uModelViewMatrix*vec4(Vertex.xyz, 1.);


 vec4 ndcPos = uProjectionMatrix * viewVertex;
 ndcPos.xyz /= ndcPos.w;
 float fact = 1.0;
 if( ndcPos.x > 0.0 && abs( uHalton.z ) == 1.0 )
   fact = 0.0;
 jitteredProjection = uProjectionMatrix;
 float doPerp = jitteredProjection[3][3] == 0.0 ? 1.0 : 0.0;
 float doOrtho = doPerp == 1.0 ? 0.0 : 1.0;
 vec2 halt = (abs(uHalton.z) == 2.0 ? 1.0 : 0.0) * fact * ( uHalton.xy / RenderSize.xy );
 // persp
 jitteredProjection[2][0] += doPerp * halt.x;
 jitteredProjection[2][1] += doPerp * halt.y;
 // ortho
 jitteredProjection[3][0] += doOrtho * halt.x;
 jitteredProjection[3][1] += doOrtho * halt.y;

gl_Position = jitteredProjection*viewVertex;
vViewVertex = viewVertex.rgba;
}