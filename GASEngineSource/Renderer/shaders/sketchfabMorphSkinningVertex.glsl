#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME PBR(02_Default_0)


attribute vec3 Vertex;
attribute vec3 Normal;
attribute vec3 Normal_0;
attribute vec3 Normal_1;
attribute vec3 Normal_2;
attribute vec3 Normal_3;
attribute vec3 Vertex_0;
attribute vec3 Vertex_1;
attribute vec3 Vertex_2;
attribute vec3 Vertex_3;
attribute vec4 Bones;
attribute vec4 Weights;

uniform float uPointSize;
uniform mat3 uModelNormalMatrix;
uniform mat3 uModelViewNormalMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec2 RenderSize;
uniform vec4 uBones[120];
uniform vec4 uHalton;
uniform vec4 uTargetWeights;

varying vec3 vViewNormal;
varying vec3 vModelVertex;
varying vec3 vModelNormal;
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

vec3 morphTransform( const in vec4 weights,  const in vec3 vertex, const in vec3 target0, const in vec3 target1, const in vec3 target2, const in vec3 target3 ) { 
	vec3 vecOut = vertex * (1.0 - ( weights[0] + weights[1] + weights[2] + weights[3]));
	vecOut += target0 * weights[0];
	vecOut += target1 * weights[1];
	vecOut += target2 * weights[2];
	vecOut += target3 * weights[3];
	return vecOut;
}

// the approximation :
// http://chilliant.blogspot.fr/2012/08/srgb-approximations-for-hlsl.html
// introduced slightly darker colors and more slight banding in the darks.
// The reference implementation (or even a single pow approx) did not introduced these effects.

// so for now we stick with the reference implementation :
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_sRGB.txt
// with the slight changes :
// - we always assume the color is >= 0.0 (so no check)
// - unlike the previous approximation, linear to srgb is monotonic so we don't need to check if the color is > 1

#define LIN_SRGB(x) x < 0.0031308 ? x * 12.92 : 1.055 * pow(x, 1.0/2.4) - 0.055
float linearTosRGB(const in float c) {
    return LIN_SRGB(c);
}
vec3 linearTosRGB(const in vec3 c) {
    return vec3(LIN_SRGB(c.r), LIN_SRGB(c.g), LIN_SRGB(c.b));
}
vec4 linearTosRGB(const in vec4 c) {
    return vec4(LIN_SRGB(c.r), LIN_SRGB(c.g), LIN_SRGB(c.b), c.a);
}

#define SRGB_LIN(x) x < 0.04045 ? x * (1.0 / 12.92) : pow((x + 0.055) * (1.0 / 1.055), 2.4)
float sRGBToLinear(const in float c) {
    return SRGB_LIN(c);
}
vec3 sRGBToLinear(const in vec3 c) {
    return vec3(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b));
}
vec4 sRGBToLinear(const in vec4 c) {
    return vec4(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b), c.a);
}

//http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec3 RGBMToRGB( const in vec4 rgba ) {
    const float maxRange = 8.0;
    return rgba.rgb * maxRange * rgba.a;
}

const mat3 LUVInverse = mat3( 6.0013,    -2.700,   -1.7995,
                              -1.332,    3.1029,   -5.7720,
                              0.3007,    -1.088,    5.6268 );

vec3 LUVToRGB( const in vec4 vLogLuv ) {
    float Le = vLogLuv.z * 255.0 + vLogLuv.w;
    vec3 Xp_Y_XYZp;
    Xp_Y_XYZp.y = exp2((Le - 127.0) / 2.0);
    Xp_Y_XYZp.z = Xp_Y_XYZp.y / vLogLuv.y;
    Xp_Y_XYZp.x = vLogLuv.x * Xp_Y_XYZp.z;
    vec3 vRGB = LUVInverse * Xp_Y_XYZp;
    return max(vRGB, 0.0);
}

// http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec4 encodeRGBM(const in vec3 col, const in float range) {
    if(range <= 0.0)
        return vec4(col, 1.0);
    vec4 rgbm;
    vec3 color = col / range;
    rgbm.a = clamp( max( max( color.r, color.g ), max( color.b, 1e-6 ) ), 0.0, 1.0 );
    rgbm.a = ceil( rgbm.a * 255.0 ) / 255.0;
    rgbm.rgb = color / rgbm.a;
    return rgbm;
}

vec3 decodeRGBM(const in vec4 col, const in float range) {
    if(range <= 0.0)
        return col.rgb;
    return range * col.rgb * col.a;
}
void main() {
// vars

vec4 viewVertex; vec3 morphVertex; vec3 skinVertex; mat4 boneMatrix; mat4 jitteredProjection; vec3 modelVertex; vec3 modelNormal; vec3 morphNormal; vec3 skinNormal; vec3 localNormal; vec3 viewNormal;

// end vars

boneMatrix = skeletalTransform( Weights, Bones );

morphVertex = morphTransform( uTargetWeights, Vertex, Vertex_0, Vertex_1, Vertex_2, Vertex_3 );

skinVertex = vec3(vec4(morphVertex.xyz, 1.)*boneMatrix);
viewVertex = uModelViewMatrix*vec4(skinVertex.xyz, 1.);
gl_PointSize = min(64.0, max(1.0, -uPointSize / viewVertex.z));


 jitteredProjection = uProjectionMatrix;
 float doPerp = jitteredProjection[3][3] == 0.0 ? 1.0 : 0.0;
 float doOrtho = doPerp == 1.0 ? 0.0 : 1.0;
 vec2 halt = ( uHalton.xy / RenderSize.xy );
 // persp
 jitteredProjection[2][0] += doPerp * halt.x;
 jitteredProjection[2][1] += doPerp * halt.y;
 // ortho
 jitteredProjection[3][0] += doOrtho * halt.x;
 jitteredProjection[3][1] += doOrtho * halt.y;

gl_Position = jitteredProjection*viewVertex;
morphNormal = morphTransform( uTargetWeights, Normal, Normal_0, Normal_1, Normal_2, Normal_3 );

skinNormal = vec3(vec4(morphNormal.xyz, 0.)*boneMatrix);
localNormal = normalize( skinNormal );

viewNormal = uModelViewNormalMatrix*localNormal.xyz;
vViewNormal = viewNormal.rgb;
vViewVertex = viewVertex.rgba;
modelNormal = uModelNormalMatrix*localNormal.xyz;
vModelNormal = modelNormal.rgb;
modelVertex = vec3(uModelMatrix*vec4(skinVertex.xyz, 1.));
vModelVertex = modelVertex.rgb;
}