precision highp float;
precision highp int;


#define MAX_DIR_LIGHTS 0
#define MAX_POINT_LIGHTS 0
#define MAX_SPOT_LIGHTS 0
#define MAX_HEMI_LIGHTS 0
#define MAX_AREA_LIGHTS 0
#define MAX_SHADOWS 0

#define GAMMA_INPUT
































#define TEXTURE_CUBE_LOD_EXT
uniform mat4 viewMatrix;
uniform vec3 cameraPosition;
uniform vec3 diffuse;
uniform float opacity;
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
#ifdef USE_COLOR
varying vec3 vColor;
#endif
#if defined( USE_MAP ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP ) || defined( USE_SPECULARMAP ) || defined( USE_REFLECTIVITYMAP ) || defined( USE_ROUGHNESSMAP ) || defined( USE_METALLICMAP ) || defined( USE_OPACITYMAP ) || defined( USE_FALLOFFMAP ) || defined( USE_TRANSLUCENCYMAP ) || defined( USE_ANISOTROPYMAP ) || defined( USE_ANISOTROPYROTATIONMAP )
varying vec2 vUv;
uniform vec4 offsetRepeat;
uniform vec4 gainBrightness;
#endif
#ifdef USE_MAP
uniform sampler2D map;
#endif
#if defined( USE_LIGHTMAP ) || defined( USE_EMISSIVEMAP )
varying vec2 vUv2;
#endif
#if defined( USE_LIGHTMAP )
uniform sampler2D lightMap;
#endif
#if defined( USE_EMISSIVEMAP )
uniform sampler2D emissiveMap;
#endif
#if defined( USE_DIFFUSEENVMAP ) || defined( USE_ENVMAP )
uniform float flipEnvMap;
#endif
#ifdef USE_ENVMAP
uniform float reflectivity;
uniform samplerCube envMap;
uniform float envMapIntensity;
uniform int combine;
uniform int envEncoding;
#endif
#ifdef USE_FOG
uniform vec3 fogColor;
#ifdef FOG_EXP2
uniform float fogDensity;
#else
uniform float fogNear;
uniform float fogFar;
#endif
#endif
#ifdef USE_SHADOWMAP
uniform sampler2D shadowMap[ MAX_SHADOWS ];
uniform vec2 shadowMapSize[ MAX_SHADOWS ];
uniform float shadowDarkness[ MAX_SHADOWS ];
uniform float shadowBias[ MAX_SHADOWS ];
varying vec4 vShadowCoord[ MAX_SHADOWS ];
float unpackDepth( const vec4 rgba_depth ) {
const vec4 bit_shift = vec4( 1.0 / ( 256.0 * 256.0 * 256.0 ), 1.0 / ( 256.0 * 256.0 ), 1.0 / 256.0, 1.0 );
float depth = dot( rgba_depth, bit_shift );
return depth;
}
#endif
#ifdef USE_SPECULARMAP
uniform sampler2D specularMap;
uniform vec4 specularGainBrightness;
uniform vec4 specularOffsetRepeat;
#endif
void main() {
gl_FragColor = vec4( diffuse, opacity );
#if defined( USE_MAP ) || defined( USE_FALLOFFMAP )
vec2 vUvLocal = applyUVOffsetRepeat( vUv, offsetRepeat );
#endif
#ifdef USE_MAP
vec4 texelColor = clamp( applyGainBrightness( texelDecode( texture2D( map, vUvLocal ), ENCODING_sRGB ), gainBrightness ), vec4(0.0), vec4(1.0) );
gl_FragColor = gl_FragColor * texelColor;
#if defined( PHYSICAL ) || defined( PHONG )
diffuseColor *= texelColor.xyz;
#endif
#endif
#ifdef ALPHATEST
if ( gl_FragColor.a < ALPHATEST ) discard;
#endif
#ifdef PHYSICAL
vec3 specularColor = specular;
#else
float specularStrength = 1.0;
#endif
#ifdef USE_SPECULARMAP
vec2 vSpecularUv = applyUVOffsetRepeat( vUv, specularOffsetRepeat );
vec4 texelSpecular = applyGainBrightness( texelDecode( texture2D( specularMap, vSpecularUv ), ENCODING_sRGB ), specularGainBrightness );
#ifdef PHYSICAL
specularColor.rgb = clamp( specularColor.rgb * texelSpecular.rgb, vec3( 0.0 ), vec3( 1.0 ) );
#else
specularStrength = clamp( specularStrength * texelSpecular.r, 0.0, 1.0 );
#endif
#endif
#ifdef USE_LIGHTMAP
#endif
#ifdef USE_COLOR
gl_FragColor = gl_FragColor * vec4( vColor, 1.0 );
#endif
#if defined( USE_ENVMAP ) && ! defined( PHYSICAL )
vec3 worldNormal = vec3( normalize( ( vec4( normal, 0.0 ) * viewMatrix ).xyz ) );
vec3 worldView = -vec3( normalize( ( vec4( viewDirection, 0.0 ) * viewMatrix ).xyz ) );
vec3 reflectVec = reflect( worldView, worldNormal );
#ifdef DOUBLE_SIDED
float flipNormal = ( -1.0 + 2.0 * float( gl_FrontFacing ) );
vec4 cubeColor = textureCube( envMap, flipNormal * vec3( flipEnvMap * reflectVec.x, reflectVec.yz ) );
#else
vec4 cubeColor = textureCube( envMap, vec3( flipEnvMap * reflectVec.x, reflectVec.yz ) );
#endif
cubeColor = texelDecode( cubeColor, envEncoding );
float fresnelReflectivity = saturate( reflectivity );
if ( combine == 1 ) {
gl_FragColor.xyz = mix( gl_FragColor.xyz, cubeColor.xyz, fresnelReflectivity );
} else if ( combine == 2 ) {
gl_FragColor.xyz += cubeColor.xyz * fresnelReflectivity;
} else {
gl_FragColor.xyz = mix( gl_FragColor.xyz, gl_FragColor.xyz * cubeColor.xyz, fresnelReflectivity );
}
#endif
#ifdef USE_SHADOWMAP
#ifdef SHADOWMAP_DEBUG
vec3 frustumColors[3];
frustumColors[0] = vec3( 1.0, 0.5, 0.0 );
frustumColors[1] = vec3( 0.0, 1.0, 0.8 );
frustumColors[2] = vec3( 0.0, 0.5, 1.0 );
#endif
#ifdef SHADOWMAP_CASCADE
int inFrustumCount = 0;
#endif
float fDepth;
vec3 shadowColor = vec3( 1.0 );
for( int i = 0; i < MAX_SHADOWS; i ++ ) {
vec3 shadowCoord = vShadowCoord[ i ].xyz / vShadowCoord[ i ].w;
bvec4 inFrustumVec = bvec4 ( shadowCoord.x >= 0.0, shadowCoord.x <= 1.0, shadowCoord.y >= 0.0, shadowCoord.y <= 1.0 );
bool inFrustum = all( inFrustumVec );
#ifdef SHADOWMAP_CASCADE
inFrustumCount += int( inFrustum );
bvec3 frustumTestVec = bvec3( inFrustum, inFrustumCount == 1, shadowCoord.z <= 1.0 );
#else
bvec2 frustumTestVec = bvec2( inFrustum, shadowCoord.z <= 1.0 );
#endif
bool frustumTest = all( frustumTestVec );
if ( frustumTest ) {
shadowCoord.z += shadowBias[ i ];
#if defined( SHADOWMAP_TYPE_PCF )
float shadow = 0.0;
const float shadowDelta = 1.0 / 9.0;
float xPixelOffset = 1.0 / shadowMapSize[ i ].x;
float yPixelOffset = 1.0 / shadowMapSize[ i ].y;
float dx0 = -1.25 * xPixelOffset;
float dy0 = -1.25 * yPixelOffset;
float dx1 = 1.25 * xPixelOffset;
float dy1 = 1.25 * yPixelOffset;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx0, dy0 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( 0.0, dy0 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx1, dy0 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx0, 0.0 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx1, 0.0 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx0, dy1 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( 0.0, dy1 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
fDepth = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx1, dy1 ) ) );
if ( fDepth < shadowCoord.z ) shadow += shadowDelta;
shadowColor = shadowColor * vec3( ( 1.0 - shadowDarkness[ i ] * shadow ) );
#elif defined( SHADOWMAP_TYPE_PCF_SOFT )
float shadow = 0.0;
float xPixelOffset = 1.0 / shadowMapSize[ i ].x;
float yPixelOffset = 1.0 / shadowMapSize[ i ].y;
float dx0 = -1.0 * xPixelOffset;
float dy0 = -1.0 * yPixelOffset;
float dx1 = 1.0 * xPixelOffset;
float dy1 = 1.0 * yPixelOffset;
mat3 shadowKernel;
mat3 depthKernel;
depthKernel[0][0] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx0, dy0 ) ) );
depthKernel[0][1] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx0, 0.0 ) ) );
depthKernel[0][2] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx0, dy1 ) ) );
depthKernel[1][0] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( 0.0, dy0 ) ) );
depthKernel[1][1] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy ) );
depthKernel[1][2] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( 0.0, dy1 ) ) );
depthKernel[2][0] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx1, dy0 ) ) );
depthKernel[2][1] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx1, 0.0 ) ) );
depthKernel[2][2] = unpackDepth( texture2D( shadowMap[ i ], shadowCoord.xy + vec2( dx1, dy1 ) ) );
vec3 shadowZ = vec3( shadowCoord.z );
shadowKernel[0] = vec3(lessThan(depthKernel[0], shadowZ ));
shadowKernel[0] *= vec3(0.25);
shadowKernel[1] = vec3(lessThan(depthKernel[1], shadowZ ));
shadowKernel[1] *= vec3(0.25);
shadowKernel[2] = vec3(lessThan(depthKernel[2], shadowZ ));
shadowKernel[2] *= vec3(0.25);
vec2 fractionalCoord = 1.0 - fract( shadowCoord.xy * shadowMapSize[i].xy );
shadowKernel[0] = mix( shadowKernel[1], shadowKernel[0], fractionalCoord.x );
shadowKernel[1] = mix( shadowKernel[2], shadowKernel[1], fractionalCoord.x );
vec4 shadowValues;
shadowValues.x = mix( shadowKernel[0][1], shadowKernel[0][0], fractionalCoord.y );
shadowValues.y = mix( shadowKernel[0][2], shadowKernel[0][1], fractionalCoord.y );
shadowValues.z = mix( shadowKernel[1][1], shadowKernel[1][0], fractionalCoord.y );
shadowValues.w = mix( shadowKernel[1][2], shadowKernel[1][1], fractionalCoord.y );
shadow = dot( shadowValues, vec4( 1.0 ) );
shadowColor = shadowColor * vec3( ( 1.0 - shadowDarkness[ i ] * shadow ) );
#else
vec4 rgbaDepth = texture2D( shadowMap[ i ], shadowCoord.xy );
float fDepth = unpackDepth( rgbaDepth );
if ( fDepth < shadowCoord.z )
shadowColor = shadowColor * vec3( 1.0 - shadowDarkness[ i ] );
#endif
}
#ifdef SHADOWMAP_DEBUG
#ifdef SHADOWMAP_CASCADE
if ( inFrustum && inFrustumCount == 1 ) gl_FragColor.xyz *= frustumColors[ i ];
#else
if ( inFrustum ) gl_FragColor.xyz *= frustumColors[ i ];
#endif
#endif
}
#ifdef GAMMA_OUTPUT
shadowColor *= shadowColor;
#endif
gl_FragColor.xyz = gl_FragColor.xyz * shadowColor;
#endif
#ifdef GAMMA_OUTPUT
gl_FragColor.xyz = sqrt( gl_FragColor.xyz );
#endif
#ifdef USE_FOG
float depth = gl_FragCoord.z / gl_FragCoord.w;
#ifdef FOG_EXP2
float fogFactor = exp2( - square( fogDensity ) * square( depth ) * LOG2 );
fogFactor = 1.0 - saturate( fogFactor );
#else
float fogFactor = smoothstep( fogNear, fogFar, depth );
#endif
gl_FragColor = mix( gl_FragColor, vec4( fogColor, gl_FragColor.w ), fogFactor );
#endif
}