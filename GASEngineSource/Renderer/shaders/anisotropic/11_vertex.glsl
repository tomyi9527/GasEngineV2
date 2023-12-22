precision highp float;
precision highp int;

#define VERTEX_TEXTURES
#define GAMMA_INPUT

#define MAX_DIR_LIGHTS 0
#define MAX_POINT_LIGHTS 0
#define MAX_SPOT_LIGHTS 4
#define MAX_HEMI_LIGHTS 0
#define MAX_AREA_LIGHTS 0
#define MAX_SHADOWS 0
#define MAX_BONES 1019




#define USE_ENVMAP











#define ANISOTROPY

#define ANISOTROPYROTATION
#define USE_ANISOTROPYROTATIONMAP





#define DOUBLE_SIDED






uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;
uniform vec3 cameraPosition;
attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;
attribute vec2 uv2;
#ifdef USE_COLOR
attribute vec3 color;
#endif
#ifdef USE_MORPHTARGETS
attribute vec3 morphTarget0;
attribute vec3 morphTarget1;
attribute vec3 morphTarget2;
attribute vec3 morphTarget3;
#ifdef USE_MORPHNORMALS
attribute vec3 morphNormal0;
attribute vec3 morphNormal1;
attribute vec3 morphNormal2;
attribute vec3 morphNormal3;
#else
attribute vec3 morphTarget4;
attribute vec3 morphTarget5;
attribute vec3 morphTarget6;
attribute vec3 morphTarget7;
#endif
#endif
#ifdef USE_SKINNING
attribute vec4 skinIndex;
attribute vec4 skinWeight;
#endif
attribute vec4 tangent;
#define PHONG
#define PHYSICAL
varying vec3 vViewPosition;
varying vec3 vTangent;
varying vec3 vBinormal;
varying vec3 vNormal;
#define PI 3.14159
#define PI2 6.28318
#define LOG2 1.442695
#define ENCODING_Linear 3000
#define ENCODING_sRGB 3001
#define ENCODING_RGBE 3002
#define ENCODING_RGBM7 3004
#define ENCODING_RGBM16 3005
#define SPECULAR_COEFF 0.18
float pow2( float a ) { return a*a; }
float square( float a ) { return a*a; }
vec2  square( vec2 a )  { return vec2( a.x*a.x, a.y*a.y ); }
vec3  square( vec3 a )  { return vec3( a.x*a.x, a.y*a.y, a.z*a.z ); }
vec4  square( vec4 a )  { return vec4( a.x*a.x, a.y*a.y, a.z*a.z, a.w*a.w ); }
float saturate( float a ) { return clamp( a, 0.0, 1.0 ); }
vec2  saturate( vec2 a )  { return clamp( a, 0.0, 1.0 ); }
vec3  saturate( vec3 a )  { return clamp( a, 0.0, 1.0 ); }
vec4  saturate( vec4 a )  { return clamp( a, 0.0, 1.0 ); }
float average( float a ) { return a; }
float average( vec2 a )  { return ( a.x + a.y) * 0.5; }
float average( vec3 a )  { return ( a.x + a.y + a.z) * 0.3333333333; }
float average( vec4 a )  { return ( a.x + a.y + a.z + a.w) * 0.25; }
float whiteCompliment( float a ) { return saturate( 1.0 - a ); }
vec2  whiteCompliment( vec2 a )  { return saturate( vec2(1.0) - a ); }
vec3  whiteCompliment( vec3 a )  { return saturate( vec3(1.0) - a ); }
vec4  whiteCompliment( vec4 a )  { return saturate( vec4(1.0) - a ); }
vec3 projectOnPlane( vec3 point, vec3 pointOnPlane, vec3 planeNormal) {
float distance = dot( planeNormal, point-pointOnPlane );
return point - distance * planeNormal;
}
float sideOfPlane( vec3 point, vec3 pointOnPlane, vec3 planeNormal ) {
return sign( dot( point - pointOnPlane, planeNormal ) );
}
vec2 applyUVOffsetRepeat( vec2 uv, vec4 offsetRepeat ) {
return uv * offsetRepeat.zw + offsetRepeat.xy;
}
vec3 linePlaneIntersect( vec3 pointOnLine, vec3 lineDirection, vec3 pointOnPlane, vec3 planeNormal ) {
return pointOnLine + lineDirection * ( dot( planeNormal, pointOnPlane - pointOnLine ) / dot( planeNormal, lineDirection ) );
}
vec4 applyGainBrightness( vec4 texel, vec4 gainBrightnessCoeff ) {
if( gainBrightnessCoeff.w < 0.0 ) {
texel.xyz = whiteCompliment( texel.xyz );
}
texel.xyz = ( texel.xyz - vec3( gainBrightnessCoeff.x ) ) * gainBrightnessCoeff.y + vec3( gainBrightnessCoeff.z + gainBrightnessCoeff.x );
return texel;
}
vec4 texelDecode( vec4 texel, int encoding ) {
if( encoding == 3001 ) {
texel = vec4( pow( max( texel.xyz, vec3( 0.0 ) ), vec3( 2.2 ) ), texel.w );
}
else if( encoding == 3002 ) {
texel = vec4( texel.xyz * pow( 2.0, texel.w*256.0 - 128.0 ), 1.0 );
}
else if( encoding == 3004 ) {
texel = vec4( texel.xyz * texel.w * 7.0, 1.0 );
}
else if( encoding == 3005 ) {
texel = vec4( texel.xyz * texel.w * 16.0, 1.0 );
}
return texel;
}
#if defined( USE_MAP ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP ) || defined( USE_SPECULARMAP ) || defined( USE_REFLECTIVITYMAP ) || defined( USE_ROUGHNESSMAP ) || defined( USE_METALLICMAP ) || defined( USE_OPACITYMAP ) || defined( USE_FALLOFFMAP ) || defined( USE_TRANSLUCENCYMAP ) || defined( USE_ANISOTROPYMAP ) || defined( USE_ANISOTROPYROTATIONMAP )
varying vec2 vUv;
#endif









#if defined( USE_LIGHTMAP ) || defined( USE_EMISSIVEMAP )
varying vec2 vUv2;
#endif
#if MAX_SPOT_LIGHTS > 0 || MAX_AREA_LIGHTS > 0 || defined( USE_BUMPMAP ) || defined( USE_ENVMAP )
varying vec3 vWorldPosition;
#endif
#ifdef USE_COLOR
varying vec3 vColor;
#endif
#ifdef USE_MORPHTARGETS
#ifndef USE_MORPHNORMALS
uniform float morphTargetInfluences[ 8 ];
#else
uniform float morphTargetInfluences[ 4 ];
#endif
#endif
#ifdef USE_SKINNING
#ifdef BONE_TEXTURE
uniform sampler2D boneTexture;
uniform int boneTextureWidth;
uniform int boneTextureHeight;
mat4 getBoneMatrix( const float i ) {
float j = i * 4.0;
float x = mod( j, float( boneTextureWidth ) );
float y = floor( j / float( boneTextureWidth ) );
float dx = 1.0 / float( boneTextureWidth );
float dy = 1.0 / float( boneTextureHeight );
y = dy * ( y + 0.5 );
vec4 v1 = texture2D( boneTexture, vec2( dx * ( x + 0.5 ), y ) );
vec4 v2 = texture2D( boneTexture, vec2( dx * ( x + 1.5 ), y ) );
vec4 v3 = texture2D( boneTexture, vec2( dx * ( x + 2.5 ), y ) );
vec4 v4 = texture2D( boneTexture, vec2( dx * ( x + 3.5 ), y ) );
mat4 bone = mat4( v1, v2, v3, v4 );
return bone;
}
#else
uniform mat4 boneGlobalMatrices[ MAX_BONES ];
mat4 getBoneMatrix( const float i ) {
mat4 bone = boneGlobalMatrices[ int(i) ];
return bone;
}
#endif
#endif
#ifdef USE_SHADOWMAP
varying vec4 vShadowCoord[ MAX_SHADOWS ];
uniform mat4 shadowMatrix[ MAX_SHADOWS ];
#endif
void main() {
#if defined( USE_MAP ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP ) || defined( USE_SPECULARMAP ) || defined( USE_REFLECTIVITYMAP ) || defined( USE_ROUGHNESSMAP ) || defined( USE_METALLICMAP ) || defined( USE_OPACITYMAP ) || defined( USE_FALLOFFMAP ) || defined( USE_TRANSLUCENCYMAP ) || defined( USE_ANISOTROPYMAP ) || defined( USE_ANISOTROPYROTATIONMAP )
vUv = uv;
#endif









#if defined( USE_LIGHTMAP ) || defined( USE_EMISSIVEMAP )
vUv2 = uv2;
#endif
#ifdef USE_COLOR
#ifdef GAMMA_INPUT
vColor = color * color;
#else
vColor = color;
#endif
#endif
#ifdef USE_MORPHNORMALS
vec3 morphedNormal = vec3( 0.0 );
morphedNormal +=  ( morphNormal0 - normal ) * morphTargetInfluences[ 0 ];
morphedNormal +=  ( morphNormal1 - normal ) * morphTargetInfluences[ 1 ];
morphedNormal +=  ( morphNormal2 - normal ) * morphTargetInfluences[ 2 ];
morphedNormal +=  ( morphNormal3 - normal ) * morphTargetInfluences[ 3 ];
morphedNormal += normal;
#endif
#ifdef USE_SKINNING
mat4 boneMatX = getBoneMatrix( skinIndex.x );
mat4 boneMatY = getBoneMatrix( skinIndex.y );
mat4 boneMatZ = getBoneMatrix( skinIndex.z );
mat4 boneMatW = getBoneMatrix( skinIndex.w );
#endif
#ifdef USE_SKINNING
mat4 skinMatrix = skinWeight.x * boneMatX;
skinMatrix 	+= skinWeight.y * boneMatY;
#ifdef USE_MORPHNORMALS
vec4 skinnedNormal = skinMatrix * vec4( morphedNormal, 0.0 );
#else
vec4 skinnedNormal = skinMatrix * vec4( normal, 0.0 );
#endif
#endif
vec3 objectNormal;
#ifdef USE_SKINNING
objectNormal = skinnedNormal.xyz;
#endif
#if !defined( USE_SKINNING ) && defined( USE_MORPHNORMALS )
objectNormal = morphedNormal;
#endif
#if !defined( USE_SKINNING ) && ! defined( USE_MORPHNORMALS )
objectNormal = normal;
#endif
#ifdef FLIP_SIDED
objectNormal = -objectNormal;
#endif
vec3 transformedNormal = normalMatrix * objectNormal;
vNormal = normalize( transformedNormal );
#ifdef USE_MORPHTARGETS
vec3 morphed = vec3( 0.0 );
morphed += ( morphTarget0 - position ) * morphTargetInfluences[ 0 ];
morphed += ( morphTarget1 - position ) * morphTargetInfluences[ 1 ];
morphed += ( morphTarget2 - position ) * morphTargetInfluences[ 2 ];
morphed += ( morphTarget3 - position ) * morphTargetInfluences[ 3 ];
#ifndef USE_MORPHNORMALS
morphed += ( morphTarget4 - position ) * morphTargetInfluences[ 4 ];
morphed += ( morphTarget5 - position ) * morphTargetInfluences[ 5 ];
morphed += ( morphTarget6 - position ) * morphTargetInfluences[ 6 ];
morphed += ( morphTarget7 - position ) * morphTargetInfluences[ 7 ];
#endif
morphed += position;
#endif
#ifdef USE_SKINNING
#ifdef USE_MORPHTARGETS
vec4 skinVertex = vec4( morphed, 1.0 );
#else
vec4 skinVertex = vec4( position, 1.0 );
#endif
vec4 skinned  = boneMatX * skinVertex * skinWeight.x;
skinned      += boneMatY * skinVertex * skinWeight.y;
skinned      += boneMatZ * skinVertex * skinWeight.z;
skinned      += boneMatW * skinVertex * skinWeight.w;
#endif
vec4 mvPosition;
#ifdef USE_SKINNING
mvPosition = modelViewMatrix * skinned;
#endif
#if !defined( USE_SKINNING ) && defined( USE_MORPHTARGETS )
mvPosition = modelViewMatrix * vec4( morphed, 1.0 );
#endif
#if !defined( USE_SKINNING ) && ! defined( USE_MORPHTARGETS )
mvPosition = modelViewMatrix * vec4( position, 1.0 );
#endif
gl_Position = projectionMatrix * mvPosition;
vViewPosition = -mvPosition.xyz;
#if defined( USE_ENVMAP ) || defined( PHONG ) || defined( LAMBERT ) || defined ( USE_SHADOWMAP )
#ifdef USE_SKINNING
vec4 worldPosition = modelMatrix * skinned;
#endif
#if defined( USE_MORPHTARGETS ) && ! defined( USE_SKINNING )
vec4 worldPosition = modelMatrix * vec4( morphed, 1.0 );
#endif
#if ! defined( USE_MORPHTARGETS ) && ! defined( USE_SKINNING )
vec4 worldPosition = modelMatrix * vec4( position, 1.0 );
#endif
#endif
#if MAX_SPOT_LIGHTS > 0 || MAX_AREA_LIGHTS > 0 || defined( USE_BUMPMAP ) || defined( USE_ENVMAP )
vWorldPosition = worldPosition.xyz;
#endif
#ifdef USE_SKINNING
vNormal = normalize( normalMatrix * skinnedNormal.xyz );
vec4 skinnedTangent = skinMatrix * vec4( tangent.xyz, 0.0 );
vTangent = normalize( normalMatrix * skinnedTangent.xyz );
#else
vNormal = normalize( normalMatrix * normal );
vTangent = normalize( normalMatrix * tangent.xyz );
#endif
vBinormal = normalize( cross( vNormal, vTangent ) * tangent.w );
#ifdef USE_SHADOWMAP
for( int i = 0; i < MAX_SHADOWS; i ++ ) {
vShadowCoord[ i ] = shadowMatrix[ i ] * worldPosition;
}
#endif
}