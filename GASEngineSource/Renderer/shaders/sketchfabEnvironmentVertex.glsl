#version 100
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
attribute vec3 Vertex;
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;


uniform vec4 uHalton;
uniform vec2 RenderSize;

varying vec3 vLocalVertex;

void main(void)
{
    vLocalVertex = Vertex.rgb;

    mat4 projectionMatrix = uProjectionMatrix;
    vec2 halt = (abs(uHalton.z) == 2.0 ? 1.0 : 0.0) * (uHalton.xy / RenderSize);
    projectionMatrix[2][0] += halt.x;
    projectionMatrix[2][1] += halt.y;

    gl_Position = (projectionMatrix * (uModelViewMatrix * vec4(Vertex, 1.0))).xyww;
}

#define SHADER_NAME Environment