precision highp float;
precision highp int;


#define MAX_DIR_LIGHTS 0
#define MAX_POINT_LIGHTS 0
#define MAX_SPOT_LIGHTS 4
#define MAX_HEMI_LIGHTS 0
#define MAX_AREA_LIGHTS 0
#define MAX_SHADOWS 0

#define GAMMA_INPUT







#define USE_ENVMAP


















#define DOUBLE_SIDED





#define TEXTURE_CUBE_LOD_EXT
uniform mat4 viewMatrix;
uniform vec3 cameraPosition;
#ifdef TEXTURE_CUBE_LOD_EXT
#extension GL_EXT_shader_texture_lod : enable
#endif
#define PHYSICAL
uniform vec3 diffuse;
uniform float opacity;
uniform vec3 ambient;
uniform vec3 emissive;
uniform vec3 falloffColor;
uniform vec4 falloffBlendParams;
uniform vec3 specular;
uniform float roughness;
uniform float metallic;
uniform float clearCoat;
uniform float clearCoatRoughness;
uniform vec3 translucency;
uniform float translucencyNormalAlpha;
uniform float translucencyNormalPower;
uniform float translucencyViewPower;
uniform float translucencyViewAlpha;
uniform float anisotropy;
uniform float anisotropyRotation;
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
#ifdef USE_FALLOFFMAP
uniform sampler2D falloffMap;
#endif
#ifdef USE_OPACITYMAP
uniform sampler2D opacityMap;
uniform vec4 opacityOffsetRepeat;
uniform vec4 opacityGainBrightness;
#endif
#ifdef USE_TRANSLUCENCYMAP
uniform sampler2D translucencyMap;
uniform vec4 translucencyOffsetRepeat;
uniform vec4 translucencyGainBrightness;
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
#if defined( USE_DIFFUSEENVMAP )
uniform samplerCube diffuseEnvMap;
uniform int diffuseEnvEncoding;
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
uniform vec3 ambientLightColor;
#if MAX_DIR_LIGHTS > 0
uniform vec3 directionalLightColor[ MAX_DIR_LIGHTS ];
uniform vec3 directionalLightDirection[ MAX_DIR_LIGHTS ];
#endif
#if MAX_HEMI_LIGHTS > 0
uniform vec3 hemisphereLightSkyColor[ MAX_HEMI_LIGHTS ];
uniform vec3 hemisphereLightGroundColor[ MAX_HEMI_LIGHTS ];
uniform vec3 hemisphereLightDirection[ MAX_HEMI_LIGHTS ];
#endif
#if MAX_POINT_LIGHTS > 0
uniform vec3 pointLightColor[ MAX_POINT_LIGHTS ];
uniform vec3 pointLightPosition[ MAX_POINT_LIGHTS ];
uniform float pointLightDistance[ MAX_POINT_LIGHTS ];
uniform float pointLightDecayExponent[ MAX_POINT_LIGHTS ];
#endif
#if MAX_SPOT_LIGHTS > 0
uniform vec3 spotLightColor[ MAX_SPOT_LIGHTS ];
uniform vec3 spotLightPosition[ MAX_SPOT_LIGHTS ];
uniform vec3 spotLightDirection[ MAX_SPOT_LIGHTS ];
uniform float spotLightAngleCos[ MAX_SPOT_LIGHTS ];
uniform float spotLightExponent[ MAX_SPOT_LIGHTS ];
uniform float spotLightDistance[ MAX_SPOT_LIGHTS ];
uniform float spotLightDecayExponent[ MAX_SPOT_LIGHTS ];
#endif
#if MAX_AREA_LIGHTS > 0
uniform vec3 areaLightColor[ MAX_AREA_LIGHTS ];
uniform vec3 areaLightPosition[ MAX_AREA_LIGHTS ];
uniform vec3 areaLightWidth[ MAX_AREA_LIGHTS ];
uniform vec3 areaLightHeight[ MAX_AREA_LIGHTS ];
uniform float areaLightDistance[ MAX_AREA_LIGHTS ];
uniform float areaLightDecayExponent[ MAX_AREA_LIGHTS ];
#endif
#if MAX_SPOT_LIGHTS > 0 || MAX_AREA_LIGHTS > 0 || defined( USE_BUMPMAP )
varying vec3 vWorldPosition;
#endif
#ifdef WRAP_AROUND
uniform vec3 wrapRGB;
#endif
varying vec3 vViewPosition;
varying vec3 vTangent;
varying vec3 vBinormal;
varying vec3 vNormal;
vec3 Fresnel_Schlick_SpecularBlendToWhite(vec3 specularColor, float hDotV) {
float Fc = pow(max( 1.0 - hDotV, 0.0 ), 5.0);
return saturate( 50.0 * average( specularColor ) ) * Fc + (1.0 - Fc) * specularColor;
}
vec3 Fresnel_Schlick_SpecularBlendToWhiteRoughness(vec3 specularColor, float hDotV, float roughness) {
float Fc = pow(max( 1.0 - hDotV, 0.0 ), 5.0) / ( 1.0 + 3.0 * roughness );
return mix( specularColor, vec3( saturate( 50.0 * average( specularColor ) ) ), Fc );
}
float Distribution_GGX( float roughness2, float nDotH ) {
float denom = nDotH * nDotH * (roughness2 - 1.0) + 1.0;
return roughness2 / ( PI * square( denom ) + 0.0000001 );
}
float Distribution_GGXAniso( vec2 anisotropicM, vec2 xyDotH, float nDotH ) {
float anisoTerm = ( xyDotH.x * xyDotH.x / ( anisotropicM.x * anisotropicM.x ) + xyDotH.y * xyDotH.y / ( anisotropicM.y * anisotropicM.y ) + nDotH * nDotH );
return 1.0 / ( PI * anisotropicM.x * anisotropicM.y * anisoTerm * anisoTerm + 0.0000001 );
}
float Visibility_Kelemen( float vDotH ) {
return 1.0 / ( 4.0 * vDotH * vDotH + 0.0000001 );
}
float Visibility_Schlick( float roughness2, float nDotL, float nDotV) {
float termL = (nDotL + sqrt(roughness2 + (1.0 - roughness2) * nDotL * nDotL));
float termV = (nDotV + sqrt(roughness2 + (1.0 - roughness2) * nDotV * nDotV));
return 1.0 / ( abs( termL * termV ) + 0.0000001 );
}
float Diffuse_Lambert() {
return 1.0 / PI;
}
float Diffuse_OrenNayer( float m2, float nDotV, float nDotL, float vDotH ) {
float termA = 1.0 - 0.5 * m2 / (m2 + 0.33);
float Cosri = 2.0 * vDotH - 1.0 - nDotV * nDotL;
float termB = 0.45 * m2 / (m2 + 0.09) * Cosri * ( Cosri >= 0.0 ? min( 1.0, nDotL / nDotV ) : nDotL );
return 1.0 / PI * ( nDotL * termA + termB );
}
mat2 createRotationMat2( float rads) {
float cos_rads = cos( rads );
float sin_rads = sin( rads );
return mat2( vec2( cos_rads, sin_rads ), vec2( -sin_rads, cos_rads ) );
}
vec2 calcAnisotropyUV( float anisotropyLocal) {
float oneMinusAbsAnisotropy = 1.0 - min( abs( anisotropyLocal ) * 0.9, 0.9 );
vec2 anisotropyUV = vec2 ( 1.0 / oneMinusAbsAnisotropy, oneMinusAbsAnisotropy );
if( anisotropy < 0.0 ) {
anisotropyUV.xy = anisotropyUV.yx;
}
return anisotropyUV;
}
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
#ifdef USE_BUMPMAP
uniform sampler2D bumpMap;
uniform vec4 bumpOffsetRepeat;
uniform float bumpScale;
vec2 dHdxy_fwd() {
#ifdef GL_OES_standard_derivatives
vec2 vBumpUv = applyUVOffsetRepeat( vUv, bumpOffsetRepeat );
vec2 dSTdx = dFdx( vBumpUv );
vec2 dSTdy = dFdy( vBumpUv );
float Hll = bumpScale * texture2D( bumpMap, vBumpUv ).x;
float dBx = bumpScale * texture2D( bumpMap, vBumpUv + dSTdx ).x - Hll;
float dBy = bumpScale * texture2D( bumpMap, vBumpUv + dSTdy ).x - Hll;
return vec2( dBx, dBy );
#else
return vec2( 0.0, 0.0 );
#endif
}
vec3 perturbNormalArb( vec3 surf_pos, vec3 surf_norm, vec2 dHdxy ) {
#ifdef GL_OES_standard_derivatives
vec3 vSigmaX = dFdx( surf_pos );
vec3 vSigmaY = dFdy( surf_pos );
vec3 vN = surf_norm;
vec3 R1 = cross( vSigmaY, vN );
vec3 R2 = cross( vN, vSigmaX );
float fDet = dot( vSigmaX, R1 );
vec3 vGrad = sign( fDet ) * ( dHdxy.x * R1 + dHdxy.y * R2 );
return normalize( abs( fDet ) * surf_norm - vGrad );
#else
return surf_norm;
#endif
}
#endif
#ifdef USE_NORMALMAP
uniform sampler2D normalMap;
uniform vec4 normalOffsetRepeat;
uniform vec2 normalScale;
vec3 perturbNormal2Arb( vec3 eye_pos, vec3 surf_norm ) {
#ifdef GL_OES_standard_derivatives
vec2 vNormalUv = applyUVOffsetRepeat( vUv, normalOffsetRepeat );
vec3 q0 = dFdx( eye_pos.xyz );
vec3 q1 = dFdy( eye_pos.xyz );
vec2 st0 = dFdx( vNormalUv.st );
vec2 st1 = dFdy( vNormalUv.st );
vec3 S = normalize(  q0 * st1.t - q1 * st0.t );
vec3 T = normalize( -q0 * st1.s + q1 * st0.s );
vec3 N = normalize( surf_norm );
vec3 NfromST = cross( S, T );
if( dot( NfromST, N ) < 0.0 ) {
S *= -1.0;
T *= -1.0;
}
vec3 mapN = texture2D( normalMap, vNormalUv ).xyz * 2.0 - 1.0;
mapN.xy = normalScale * mapN.xy;
mat3 tsn = mat3( S, T, N );
return normalize( tsn * mapN );
#else
return surf_norm;
#endif
}
#endif
#ifdef USE_SPECULARMAP
uniform sampler2D specularMap;
uniform vec4 specularGainBrightness;
uniform vec4 specularOffsetRepeat;
#endif
#ifdef USE_ANISOTROPYMAP
uniform sampler2D anisotropyMap;
uniform vec4 anisotropyGainBrightness;
uniform vec4 anisotropyOffsetRepeat;
#endif
#ifdef USE_ANISOTROPYROTATIONMAP
uniform sampler2D anisotropyRotationMap;
uniform vec4 anisotropyRotationGainBrightness;
uniform vec4 anisotropyRotationOffsetRepeat;
#endif
#ifdef USE_METALLICMAP
uniform sampler2D metallicMap;
uniform vec4 metallicGainBrightness;
uniform vec4 metallicOffsetRepeat;
#endif
#ifdef USE_ROUGHNESSMAP
uniform sampler2D roughnessMap;
uniform vec4 roughnessOffsetRepeat;
uniform vec4 roughnessGainBrightness;
#endif

float calcLightAttenuation( float lightDistance, float cutoffDistance, float decayExponent ) {
if ( decayExponent > 0.0 && cutoffDistance > 0.0 ) {
return pow( saturate( -lightDistance / cutoffDistance + 1.0 ), decayExponent );
}
else if ( decayExponent < 0.0 ) {
float numerator = 1.0;
if( cutoffDistance > 0.0 ) {
numerator = ( saturate( 1.0 - pow( lightDistance / cutoffDistance, 4.0 ) ) );
numerator *= numerator;
} 
return numerator / ( ( lightDistance * lightDistance ) + 1.0 );
}
else {
return 1.0;
}
}
void main() {
gl_FragColor = vec4( vec3 ( 0.0 ), opacity );
vec3 diffuseColor = diffuse;
vec3 translucencyColor = translucency;
vec3 normal = normalize( vNormal );
vec3 viewDirection = normalize( vViewPosition );
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
#ifdef USE_OPACITYMAP
vec2 vOpacityUv = applyUVOffsetRepeat( vUv, opacityOffsetRepeat );
vec4 texelOpacity = applyGainBrightness( texture2D( opacityMap, vOpacityUv ), opacityGainBrightness );
gl_FragColor.w = clamp( gl_FragColor.w * texelOpacity.r, 0.0, 1.0 );
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
#ifdef USE_ANISOTROPYMAP
vec2 vAnisotropyUv = applyUVOffsetRepeat( vUv, anisotropyOffsetRepeat );
#else
#ifdef ANISOTROPY
#if defined( USE_MAP ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP ) || defined( USE_SPECULARMAP ) || defined( USE_REFLECTIVITYMAP ) || defined( USE_ROUGHNESSMAP ) || defined( USE_METALLICMAP ) || defined( USE_OPACITYMAP ) || defined( USE_FALLOFFMAP ) || defined( USE_TRANSLUCENCYMAP ) || defined( USE_ANISOTROPYMAP ) || defined( USE_ANISOTROPYROTATIONMAP )
vec2 vAnisotropyUv = vUv;
#else
vec2 vAnisotropyUv = vec2( 0, 0 );
#endif
#endif
#endif
float anisotropyStrength = anisotropy;
#ifdef USE_ANISOTROPYMAP
vec4 texelAnisotropy = applyGainBrightness( texture2D( anisotropyMap, vAnisotropyUv ), anisotropyGainBrightness );
anisotropyStrength = clamp( anisotropyStrength + texelAnisotropy.r, -1.0, 1.0 );
#endif
float anisotropyRotationStrength = anisotropyRotation;
#ifdef USE_ANISOTROPYROTATIONMAP
vec2 vAnisotropyRotationUv = applyUVOffsetRepeat( vUv, anisotropyRotationOffsetRepeat );
vec4 texelAnisotropyRotation = applyGainBrightness( texture2D( anisotropyRotationMap, vAnisotropyRotationUv ), anisotropyRotationGainBrightness );
anisotropyRotationStrength += texelAnisotropyRotation.r;
#endif
float roughnessStrength = roughness;
#ifdef USE_ROUGHNESSMAP
vec2 vRoughnessUv = applyUVOffsetRepeat( vUv, roughnessOffsetRepeat );
vec4 texelRoughness = applyGainBrightness( texture2D( roughnessMap, vRoughnessUv ), roughnessGainBrightness );
roughnessStrength = clamp( roughnessStrength * texelRoughness.g, 0.0, 1.0 );
#endif
float metallicStrength = metallic;
#ifdef USE_METALLICMAP
vec2 vMetallicUv = applyUVOffsetRepeat( vUv, metallicOffsetRepeat );
vec4 texelMetallic = applyGainBrightness( texture2D( metallicMap, vMetallicUv ), metallicGainBrightness );
metallicStrength = clamp( metallicStrength * texelMetallic.b, 0.0, 1.0 );
#endif
#ifdef USE_TRANSLUCENCYMAP
vec2 vTranslucencyUv = applyUVOffsetRepeat( vUv, translucencyOffsetRepeat );
vec4 texelTranslucency = applyGainBrightness( texture2D( translucencyMap, vTranslucencyUv ), translucencyGainBrightness );
translucencyColor.xyz = clamp( translucencyColor.xyz * texelTranslucency.xyz, vec3( 0.0 ), vec3( 1.0 ) );
#endif

mat3 tsb = mat3( normalize( vTangent ), normalize( vBinormal ), normal );
#ifdef USE_NORMALMAP
normal = perturbNormal2Arb( -vViewPosition, normal );
#endif
#if defined( USE_BUMPMAP )
normal = perturbNormalArb( -vViewPosition, normal, dHdxy_fwd() );
#endif
#ifdef DOUBLE_SIDED
normal = normal * ( -1.0 + 2.0 * float( gl_FrontFacing ) );
#endif
#ifdef FALLOFF
vec3 modulatedFalloffColor = falloffColor;
#ifdef USE_FALLOFFMAP
vec4 falloffTexelColor = texelDecode( texture2D( falloffMap, vUvLocal ), ENCODING_sRGB );
modulatedFalloffColor = modulatedFalloffColor * falloffTexelColor.xyz;
#endif
float fm = abs( dot( normal, viewDirection ) );
fm = /*falloffBlendParams.x * fm + falloffBlendParams.y * */ ( fm * fm * ( 3.0 - 2.0 * fm ) );
diffuseColor = mix( modulatedFalloffColor, diffuseColor, fm );
#endif
float nDotV = saturate( dot( normal, viewDirection ) );
float m2 = pow( clamp( roughnessStrength, 0.02, 1.0 ), 4.0 );
float m2ClearCoat = pow( clamp( clearCoatRoughness, 0.02, 1.0 ), 4.0 );
specularColor = mix( specularColor * SPECULAR_COEFF, diffuseColor, metallicStrength );
diffuseColor *= ( 1.0 - metallicStrength );
#ifdef ANISOTROPY
vec2 anisotropicM = calcAnisotropyUV( anisotropyStrength ) * sqrt( m2 );
#ifdef ANISOTROPYROTATION
mat2 anisotropicRotationMatrix = createRotationMat2( anisotropyRotationStrength * 2.0 * PI );
#endif
vec3 anisotropicS = tsb[1];
vec3 anisotropicT = tsb[0];
#endif
vec3 totalLighting  = vec3( 0.0 );
#if ( defined( USE_ENVMAP ) || defined( USE_DIFFUSEENVMAP ) ) && defined( PHYSICAL )
{
vec3 worldNormal = vec3( normalize( ( vec4( normal, 0.0 ) * viewMatrix ).xyz ) );
vec3 worldView = -vec3( normalize( ( vec4( viewDirection, 0.0 ) * viewMatrix ).xyz ) );
vec3 reflectVec = reflect( worldView, worldNormal );
vec3 hVector = normal;//normalize( viewDirection.xyz + lVector.xyz );
float nDotH = saturate( dot( normal, normal ) );
float hDotV = saturate( dot( normal, viewDirection ) );
float nDotL = hDotV;//saturate( dot( normal, lVector ) );
vec3 queryVector = vec3( flipEnvMap * reflectVec.x, reflectVec.yz );
#ifdef DOUBLE_SIDED
queryVector *= ( -1.0 + 2.0 * float( gl_FrontFacing ) );
#endif
vec3 worldEnvNormal = vec3( normalize( ( vec4( normal, 0.0 ) * viewMatrix ).xyz ) );
worldEnvNormal = vec3( flipEnvMap * worldEnvNormal.x, worldEnvNormal.yz );
#ifdef DOUBLE_SIDED
worldEnvNormal *= ( -1.0 + 2.0 * float( gl_FrontFacing ) );
#endif
vec4 diffuseEnvColor = vec4( 0.0, 0.0, 0.0, 1.0 );
#if defined( USE_DIFFUSEENVMAP )
diffuseEnvColor = texelDecode( textureCube( diffuseEnvMap, worldEnvNormal ), diffuseEnvEncoding );
#elif defined( USE_ENVMAP )
#if defined( TEXTURE_CUBE_LOD_EXT )
diffuseEnvColor = texelDecode( textureCubeLodEXT( envMap, worldEnvNormal, 9.5 ), envEncoding );
#else
diffuseEnvColor = texelDecode( textureCube( envMap, worldEnvNormal, 10.0 ), envEncoding );
#endif
#endif
vec4 specularEnvColor = vec4( 0.0, 0.0, 0.0, 1.0 );
#if defined( USE_ENVMAP )
#if defined( TEXTURE_CUBE_LOD_EXT )
float specularMIPLevel = log2( exp2( 8.0 ) * sqrt( 3.0 ) ) - 0.5 * log2( pow2( 2.0 / pow2( roughnessStrength + 0.0001 ) - 2.0 ) + 1.0 );
specularEnvColor = texelDecode( textureCubeLodEXT( envMap, queryVector, specularMIPLevel ), envEncoding );
#else
specularEnvColor = mix( texelDecode( textureCube( envMap, queryVector ), envEncoding ), texelDecode( textureCube( envMap, queryVector, 10.0 ), envEncoding ), roughnessStrength );
#endif
#endif
vec3 specClearCoat = vec3(0, 0, 0);
#if defined( CLEARCOAT ) && defined( USE_ENVMAP )
#if defined( TEXTURE_CUBE_LOD_EXT )
float clearCoatMIPLevel = log2( exp2( 8.0 ) * sqrt( 3.0 ) ) - 0.5 * log2( pow2( 2.0 / pow2( clearCoatRoughness + 0.0001 ) - 2.0 ) + 1.0 );
vec4 specularClearCoatEnvColor = texelDecode( textureCubeLodEXT( envMap, queryVector, clearCoatMIPLevel ), envEncoding );
#else
vec4 specularClearCoatEnvColor = mix( texelDecode( textureCube( envMap, queryVector ), envEncoding ), texelDecode( textureCube( envMap, queryVector, 10.0 ), envEncoding ), clearCoatRoughness );
#endif
vec3 fresnelClearCoat = Fresnel_Schlick_SpecularBlendToWhiteRoughness( vec3( SPECULAR_COEFF ), nDotL, m2ClearCoat );
specClearCoat = specularClearCoatEnvColor.rgb * fresnelClearCoat;
#endif
vec3 fresnelColor = Fresnel_Schlick_SpecularBlendToWhiteRoughness( specularColor, nDotL, m2 );
vec3 spec = fresnelColor * specularEnvColor.rgb;
vec3 diff = diffuseColor * diffuseEnvColor.rgb;
vec3 shadingResult = spec + diff;
#ifdef CLEARCOAT
shadingResult = mix( shadingResult, specClearCoat, clearCoat );
#endif
totalLighting  += shadingResult * envMapIntensity;
}
#endif
#if MAX_POINT_LIGHTS > 0
for ( int i = 0; i < MAX_POINT_LIGHTS; i ++ ) {
vec4 lPosition = viewMatrix * vec4( pointLightPosition[ i ], 1.0 );
vec3 lVector = lPosition.xyz + vViewPosition.xyz;
float distanceAttenuation = calcLightAttenuation( length( lVector ), pointLightDistance[ i ], pointLightDecayExponent[i] );
vec3 incidentLight = pointLightColor[ i ] * distanceAttenuation;
lVector = normalize( lVector );
vec3 hVector = normalize( viewDirection.xyz + lVector.xyz );
float nDotH = saturate( dot( normal, hVector ) );
float nDotL = saturate( dot( normal, lVector ) );
float hDotV = saturate( dot( hVector, viewDirection ) );
#ifdef CLEARCOAT
float dClearCoat = Distribution_GGX( m2ClearCoat, nDotH );
float visClearCoat = Visibility_Kelemen( hDotV );
vec3 fresnelClearCoat = Fresnel_Schlick_SpecularBlendToWhite( vec3( SPECULAR_COEFF ), hDotV );
vec3 specClearCoat = clamp( nDotL * dClearCoat * visClearCoat, 0.0, 10.0 ) * fresnelClearCoat;
#endif
#ifdef ANISOTROPY
vec2 xyDotH = vec2( dot( anisotropicS, hVector ), dot( anisotropicT, hVector ) );
#ifdef ANISOTROPYROTATION
xyDotH = anisotropicRotationMatrix * xyDotH;
#endif
float d = Distribution_GGXAniso( anisotropicM, xyDotH, nDotH );
#else
float d = Distribution_GGX( m2, nDotH );
#endif
float vis = Visibility_Schlick(m2, nDotL, nDotV);
vec3 fresnelColor = Fresnel_Schlick_SpecularBlendToWhite( specularColor, hDotV );
vec3 spec = clamp( nDotL * d * vis, 0.0, 10.0 ) * fresnelColor;
vec3 diff = nDotL * Diffuse_Lambert() * diffuseColor;
#ifdef TRANSLUCENCY
diff *= whiteCompliment( translucencyColor.xyz );
#endif
vec3 shadingResult = spec + diff;
#ifdef CLEARCOAT
shadingResult = mix( shadingResult, specClearCoat, clearCoat );
#endif
totalLighting  += incidentLight * shadingResult;
#ifdef TRANSLUCENCY
float lightNormalTL = mix( 1.0, pow( abs( dot( lVector.xyz, normal ) ), translucencyNormalPower ), translucencyNormalAlpha );
float viewNormalTL = mix( 1.0, pow( abs( dot( viewDirection.xyz, lVector.xyz) ), translucencyViewPower ), translucencyViewAlpha );
totalLighting += lightNormalTL * viewNormalTL * translucencyColor.rgb * incidentLight;
#endif
}
#endif
#if MAX_SPOT_LIGHTS > 0
for ( int i = 0; i < MAX_SPOT_LIGHTS; i ++ ) {
vec4 lPosition = viewMatrix * vec4( spotLightPosition[ i ], 1.0 );
vec3 lVector = lPosition.xyz + vViewPosition.xyz;
float distanceAttenuation = calcLightAttenuation( length( lVector ), spotLightDistance[ i ], spotLightDecayExponent[i] );
vec3 incidentLight = spotLightColor[ i ] * distanceAttenuation;
lVector = normalize( lVector );
float spotEffect = dot( spotLightDirection[ i ], normalize( spotLightPosition[ i ] - vWorldPosition ) );
if ( spotEffect > spotLightAngleCos[ i ] ) {
spotEffect = pow( max( spotEffect, 0.0 ), spotLightExponent[ i ] );
incidentLight *= spotEffect;
vec3 hVector = normalize( viewDirection.xyz + lVector.xyz );
float nDotH = saturate( dot( normal, hVector ) );
float nDotL = saturate( dot( normal, lVector ) );
float hDotV = saturate( dot( hVector, viewDirection ) );
#ifdef CLEARCOAT
float dClearCoat = Distribution_GGX( m2ClearCoat, nDotH );
float visClearCoat = Visibility_Kelemen( hDotV );
vec3 fresnelClearCoat = Fresnel_Schlick_SpecularBlendToWhite( vec3( SPECULAR_COEFF ), hDotV );
vec3 specClearCoat = clamp( nDotL * dClearCoat * visClearCoat, 0.0, 10.0 ) * fresnelClearCoat;
#endif
#ifdef ANISOTROPY
vec2 xyDotH = vec2( dot( anisotropicS, hVector ), dot( anisotropicT, hVector ) );
#ifdef ANISOTROPYROTATION
xyDotH = anisotropicRotationMatrix * xyDotH;
#endif
float d = Distribution_GGXAniso( anisotropicM, xyDotH, nDotH );
#else
float d = Distribution_GGX( m2, nDotH );
#endif
float vis = Visibility_Schlick(m2, nDotL, nDotV);
vec3 fresnelColor = Fresnel_Schlick_SpecularBlendToWhite( specularColor, hDotV );
vec3 spec = clamp( nDotL * d * vis, 0.0, 10.0 ) * fresnelColor;
vec3 diff = nDotL * Diffuse_Lambert() * diffuseColor;
#ifdef TRANSLUCENCY
diff *= whiteCompliment( translucencyColor.xyz );
#endif
vec3 shadingResult = spec + diff;
#ifdef CLEARCOAT
shadingResult = mix( shadingResult, specClearCoat, clearCoat );
#endif
totalLighting  += incidentLight * shadingResult;
#ifdef TRANSLUCENCY
float lightNormalTL = mix( 1.0, pow( abs( dot( lVector.xyz, normal ) ), translucencyNormalPower ), translucencyNormalAlpha );
float viewNormalTL = mix( 1.0, pow( abs( dot( viewDirection.xyz, lVector.xyz) ), translucencyViewPower ), translucencyViewAlpha );
totalLighting += lightNormalTL * viewNormalTL * translucencyColor.rgb * incidentLight;
#endif
}
}
#endif
#if MAX_DIR_LIGHTS > 0
for( int i = 0; i < MAX_DIR_LIGHTS; i ++ ) {
vec4 lDirection = viewMatrix * vec4( directionalLightDirection[ i ], 0.0 );
vec3 lVector = normalize( lDirection.xyz );
vec3 incidentLight = directionalLightColor[ i ];
vec3 hVector = normalize( viewDirection.xyz + lVector.xyz );
float nDotH = saturate( dot( normal, hVector ) );
float nDotL = saturate( dot( normal, lVector ) );
float hDotV = saturate( dot( hVector, viewDirection ) );
#ifdef CLEARCOAT
float dClearCoat = Distribution_GGX( m2ClearCoat, nDotH );
float visClearCoat = Visibility_Kelemen( hDotV );
vec3 fresnelClearCoat = Fresnel_Schlick_SpecularBlendToWhite( vec3( SPECULAR_COEFF ), hDotV );
vec3 specClearCoat = clamp( nDotL * dClearCoat * visClearCoat, 0.0, 10.0 ) * fresnelClearCoat;
#endif
#ifdef ANISOTROPY
vec2 xyDotH = vec2( dot( anisotropicS, hVector ), dot( anisotropicT, hVector ) );
#ifdef ANISOTROPYROTATION
xyDotH = anisotropicRotationMatrix * xyDotH;
#endif
float d = Distribution_GGXAniso( anisotropicM, xyDotH, nDotH );
#else
float d = Distribution_GGX( m2, nDotH );
#endif
float vis = Visibility_Schlick(m2, nDotL, nDotV);
vec3 fresnelColor = Fresnel_Schlick_SpecularBlendToWhite( specularColor, hDotV );
vec3 spec = clamp( nDotL * d * vis, 0.0, 10.0 ) * fresnelColor;
vec3 diff = nDotL * Diffuse_Lambert() * diffuseColor;
#ifdef TRANSLUCENCY
diff *= whiteCompliment( translucencyColor.xyz );
#endif
vec3 shadingResult = spec + diff;
#ifdef CLEARCOAT
shadingResult = mix( shadingResult, specClearCoat, clearCoat );
#endif
totalLighting  += incidentLight * shadingResult;
#ifdef TRANSLUCENCY
float lightNormalTL = mix( 1.0, pow( abs( dot( lVector.xyz, normal ) ), translucencyNormalPower ), translucencyNormalAlpha );
float viewNormalTL = mix( 1.0, pow( abs( dot( viewDirection.xyz, lVector.xyz) ), translucencyViewPower ), translucencyViewAlpha );
totalLighting += lightNormalTL * viewNormalTL * translucencyColor.rgb * incidentLight;
#endif
}
#endif
#if MAX_HEMI_LIGHTS > 0
for( int i = 0; i < MAX_HEMI_LIGHTS; i ++ ) {
vec4 lDirection = viewMatrix * vec4( hemisphereLightDirection[ i ], 0.0 );
vec3 lVector = normalize( lDirection.xyz );
float nDotL = dot( normal, lVector );
vec3 hemiColor = ( PI / 2.0 ) * ( ( 1.0 + nDotL ) * hemisphereLightSkyColor[ i ] + ( 1.0 - nDotL ) * hemisphereLightGroundColor[ i ] );
totalLighting += diffuseColor * hemiColor;
}
#endif
#if MAX_AREA_LIGHTS > 0
for( int i = 0; i < MAX_AREA_LIGHTS; i ++ ) {
vec3 lPosition = ( viewMatrix * vec4( areaLightPosition[ i ], 1.0 ) ).xyz;
vec3 width = areaLightWidth[ i ];
vec3 height = areaLightHeight[ i ];
vec3 up = normalize( ( viewMatrix * vec4( height, 0.0 ) ).xyz );
vec3 right = normalize( ( viewMatrix * vec4( width, 0.0 ) ).xyz );
vec3 pnormal = normalize( cross( right, up ) );
float widthScalar = length( width );
float heightScalar = length( height );
vec3 projection = projectOnPlane( -vViewPosition.xyz, lPosition, pnormal );
vec3 dir = projection - lPosition;
vec2 diagonal = vec2( dot( dir, right ), dot( dir, up ) );
vec2 nearest2D = vec2( clamp( diagonal.x, -widthScalar, widthScalar ), clamp( diagonal.y, -heightScalar, heightScalar ) );
vec3 nearestPointInside = lPosition + ( right *nearest2D.x + up * nearest2D.y );
vec3 lVector = ( nearestPointInside + vViewPosition.xyz );
float distanceAttenuation = calcLightAttenuation( length( lVector ), areaLightDistance[ i ], areaLightDecayExponent[i] );
lVector = normalize( lVector );
vec3 incidentLight = areaLightColor[ i ] * distanceAttenuation * 0.01;
float nDotLDiffuse = saturate( dot( normal, lVector ) );
vec3 diff = Diffuse_Lambert() * diffuseColor * widthScalar * heightScalar;
vec3 viewReflection = reflect( viewDirection.xyz, normal );
vec3 reflectionLightPlaneIntersection = linePlaneIntersect( -vViewPosition.xyz, viewReflection, lPosition, pnormal );
float specAngle = dot( viewReflection, pnormal );
if ( specAngle < 0.0 ) {
vec3 dirSpec = reflectionLightPlaneIntersection - lPosition;
vec2 dirSpec2D = vec2( dot( dirSpec, right ), dot( dirSpec, up ) );
vec2 nearestSpec2D = vec2( clamp( dirSpec2D.x, -widthScalar, widthScalar ), clamp( dirSpec2D.y, -heightScalar, heightScalar ) );
lVector = normalize( lPosition + ( right *nearestSpec2D.x + up * nearestSpec2D.y ) + vViewPosition.xyz );
} else { 
lVector = vec3( 0 );
}
vec3 hVector = normalize( viewDirection.xyz + lVector.xyz );
float nDotH = saturate( dot( normal, hVector ) );
float nDotL = saturate( dot( normal, lVector ) );
float hDotV = saturate( dot( hVector, viewDirection ) );
#ifdef CLEARCOAT
float dClearCoat = Distribution_GGX( m2ClearCoat, nDotH );
float visClearCoat = Visibility_Kelemen( hDotV );
vec3 fresnelClearCoat = Fresnel_Schlick_SpecularBlendToWhite( vec3( SPECULAR_COEFF ), hDotV );
vec3 specClearCoat = clamp( nDotL * dClearCoat * visClearCoat, 0.0, 10.0 ) * fresnelClearCoat;
#endif
#ifdef TRANSLUCENCY
diff *= whiteCompliment( translucencyColor.xyz );
#endif
#ifdef CLEARCOAT
diff = mix( diff, specClearCoat, clearCoat );
#endif
#ifdef ANISOTROPY
vec2 xyDotH = vec2( dot( anisotropicS, hVector ), dot( anisotropicT, hVector ) );
#ifdef ANISOTROPYROTATION
xyDotH = anisotropicRotationMatrix * xyDotH;
#endif
float d = Distribution_GGXAniso( anisotropicM, xyDotH, nDotH );
#else
float d = Distribution_GGX( m2, nDotH );
#endif
float vis = Visibility_Schlick(m2, nDotL, nDotV);
vec3 fresnelColor = Fresnel_Schlick_SpecularBlendToWhite( specularColor, hDotV );
vec3 spec = clamp( nDotL * d * vis, 0.0, 10.0 ) * fresnelColor;
totalLighting  += incidentLight * spec;
totalLighting  += incidentLight * nDotLDiffuse * diff;
#ifdef TRANSLUCENCY
float lightNormalTL = mix( 1.0, pow( abs( dot( lVector.xyz, normal ) ), translucencyNormalPower ), translucencyNormalAlpha );
float viewNormalTL = mix( 1.0, pow( abs( dot( viewDirection.xyz, lVector.xyz) ), translucencyViewPower ), translucencyViewAlpha );
totalLighting += lightNormalTL * viewNormalTL * translucencyColor.rgb * incidentLight;
#endif
}
#endif
#ifdef CLEARCOAT
totalLighting += diffuseColor * ( ambientLightColor * ( 1.0 - clearCoat ) );
#else
totalLighting += diffuseColor * ambientLightColor;
#endif
gl_FragColor.xyz += totalLighting;
vec3 emissiveLocal = emissive;
#ifdef USE_EMISSIVEMAP
vec3 emissiveColor = texture2D( emissiveMap, vUv2 ).xyz;
#ifdef GAMMA_INPUT
emissiveColor *= emissiveColor;
#endif
emissiveLocal *= emissiveColor;
#endif
gl_FragColor.xyz += emissiveLocal;
vec3 ambientLocal = ambient;
#ifdef USE_LIGHTMAP
vec3 ambientColor = texture2D( lightMap, vUv2 ).xyz;
#ifdef GAMMA_INPUT
ambientColor *= ambientColor;
#endif
ambientLocal *= ambientColor;
#ifdef CLEARCOAT
ambientLocal *= ( 1.0 - clearCoat );
#endif
#endif
gl_FragColor.xyz += diffuseColor * ambientLocal;
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
gl_FragColor.xyz *= gl_FragColor.w;
}