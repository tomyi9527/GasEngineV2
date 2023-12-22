#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME EarlyZ13


attribute vec3 Vertex;
attribute vec4 Bones;
attribute vec4 Weights;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec2 RenderSize;
uniform vec4 uBones[60];
uniform vec4 uHalton;

varying vec4 vViewVertex;


//////////////////////////////
// OPTIMIZED VERSION (NO IF)
//////////////////////////////
mat4 skeletalTransform( const in vec4 weightsVec, const in vec4 bonesIdx ) {
    mat4 outMat_1;
    mat4 tmpMat_2;
    highp ivec4 tmpvar_3;
    tmpvar_3 = (3 * ivec4(bonesIdx));
    tmpMat_2 = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    vec4 tmpvar_4;
    tmpvar_4 = -(abs(weightsVec));
    tmpMat_2[0] = uBones[tmpvar_3.x];
    tmpMat_2[1] = uBones[(tmpvar_3.x + 1)];
    tmpMat_2[2] = uBones[(tmpvar_3.x + 2)];
    outMat_1 = ((float(
    ((tmpvar_4.x + tmpvar_4.y) >= -((tmpvar_4.z + tmpvar_4.w)))
    ) * mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0)) + (weightsVec.x * tmpMat_2));
    tmpMat_2[0] = uBones[tmpvar_3.y];
    tmpMat_2[1] = uBones[(tmpvar_3.y + 1)];
    tmpMat_2[2] = uBones[(tmpvar_3.y + 2)];
    outMat_1 = (outMat_1 + (weightsVec.y * tmpMat_2));
    tmpMat_2[0] = uBones[tmpvar_3.z];
    tmpMat_2[1] = uBones[(tmpvar_3.z + 1)];
    tmpMat_2[2] = uBones[(tmpvar_3.z + 2)];
    outMat_1 = (outMat_1 + (weightsVec.z * tmpMat_2));
    tmpMat_2[0] = uBones[tmpvar_3.w];
    tmpMat_2[1] = uBones[(tmpvar_3.w + 1)];
    tmpMat_2[2] = uBones[(tmpvar_3.w + 2)];
    outMat_1 = (outMat_1 + (weightsVec.w * tmpMat_2));

    return outMat_1;
}

//////////////////////////////
// UN-OPTIMIZED VERSION (WITH IF)
//////////////////////////////

// //http://http.developer.nvidia.com/GPUGems/gpugems_ch04.html
// mat4 getMat4FromVec4( const int index, inout mat4 myMat ) {
//     // We have to use a global variable because we can't access dynamically
//     // matrix is transpose so we should do vec * matrix
//     myMat[0] = uBones[ index ];
//     myMat[1] = uBones[ index + 1];
//     myMat[2] = uBones[ index + 2];
//     return myMat;
// }

// mat4 skeletalTransform( const in vec4 weightsVec, const in vec4 bonesIdx ) {
//     ivec4 idx =  3 * ivec4(bonesIdx);
//     mat4 tmpMat = mat4(1.0);
//     mat4 outMat = mat4(0.0);

//     // we handle negative weights
//     if(all(equal(weightsVec, vec4(0.0)))) return tmpMat;

//     if(weightsVec.x != 0.0) outMat += weightsVec.x * getMat4FromVec4( idx.x, tmpMat );
//     if(weightsVec.y != 0.0) outMat += weightsVec.y * getMat4FromVec4( idx.y, tmpMat );
//     if(weightsVec.z != 0.0) outMat += weightsVec.z * getMat4FromVec4( idx.z, tmpMat );
//     if(weightsVec.w != 0.0) outMat += weightsVec.w * getMat4FromVec4( idx.w, tmpMat );
//     return outMat;
// }

//////////////////////////////
// UN-OPTIMIZED VERSION (NO IF)
//////////////////////////////

// mat4 skeletalTransform( const in vec4 weightsVec, const in vec4 bonesIdx ) {
//     ivec4 idx =  3 * ivec4(bonesIdx);
//     mat4 tmpMat = mat4(1.0);

//     // if sum is 0, return identity
//     vec4 absWeights = -abs(weightsVec);
//     mat4 outMat = step(0.0, absWeights.x + absWeights.y + absWeights.z + absWeights.w) * tmpMat;

//     // we handle negative weights
//     // outMat[3][3] += weightsVec.x + weightsVec.y + weightsVec.z + weightsVec.w;

//     tmpMat[0] = uBones[ idx.x ];
//     tmpMat[1] = uBones[ idx.x + 1];
//     tmpMat[2] = uBones[ idx.x + 2];
//     outMat += weightsVec.x * tmpMat;

//     tmpMat[0] = uBones[ idx.y ];
//     tmpMat[1] = uBones[ idx.y + 1];
//     tmpMat[2] = uBones[ idx.y + 2];
//     outMat += weightsVec.y * tmpMat;

//     tmpMat[0] = uBones[ idx.z ];
//     tmpMat[1] = uBones[ idx.z + 1];
//     tmpMat[2] = uBones[ idx.z + 2];
//     outMat += weightsVec.z * tmpMat;

//     tmpMat[0] = uBones[ idx.w ];
//     tmpMat[1] = uBones[ idx.w + 1];
//     tmpMat[2] = uBones[ idx.w + 2];
//     outMat += weightsVec.w * tmpMat;

//     return outMat;
// }

void main() {
// vars

const float floatWhite = float(1.0); mat4 jitteredProjection; vec4 viewVertex; vec3 skinVertex; mat4 boneMatrix;

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

boneMatrix = skeletalTransform( Weights, Bones );

skinVertex = vec3(vec4(Vertex.xyz, 1.)*boneMatrix);
viewVertex = uModelViewMatrix*vec4(skinVertex.xyz, 1.);
gl_Position = jitteredProjection*viewVertex;
vViewVertex = viewVertex.rgba;
}