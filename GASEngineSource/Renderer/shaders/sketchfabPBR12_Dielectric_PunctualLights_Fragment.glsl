#version 100
#extension GL_OES_standard_derivatives : enable
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME PBR12
#define _PCFx1


uniform float uAOPBRFactor;
uniform float uAlbedoPBRFactor;
uniform float uBumpMapFactor;
uniform float uCavityPBRFactor;
uniform float uDiffuseColorFactor;
uniform float uDiffuseIntensityFactor;
uniform float uDiffusePBRFactor;
uniform float uDisplacementFactor;
uniform float uDisplacementNormal;
uniform float uEmitColorFactor;
uniform float uGlossinessPBRFactor;
uniform float uMetalnessPBRFactor;
uniform float uNormalMapFactor;
uniform float uOpacityFactor;
uniform float uRGBMRange;
uniform float uReflection;
uniform float uRoughnessPBRFactor;
uniform float uShadowReceive0_bias;
uniform float uShadowReceive0_normalBias;
uniform float uShadowReceive1_bias;
uniform float uShadowReceive1_normalBias;
uniform float uSketchfabLight0_spotBlend;
uniform float uSketchfabLight0_spotCutOff;
uniform float uSketchfabLight1_spotBlend;
uniform float uSketchfabLight1_spotCutOff;
uniform float uSketchfabLight2_spotBlend;
uniform float uSketchfabLight2_spotCutOff;
uniform float uSpecularColorFactor;
uniform float uSpecularF0Factor;
uniform float uSpecularHardnessFactor;
uniform float uSpecularPBRFactor;
uniform int uAOPBROccludeSpecular;
uniform int uDrawOpaque;
uniform int uEmitMultiplicative;
uniform int uNormalMapFlipY;
uniform int uOpacityAdditive;
uniform int uOpacityDithering;
uniform int uOpacityInvert;
uniform int uOutputLinear;
uniform mat4 uShadow_Texture0_projectionMatrix;
uniform mat4 uShadow_Texture0_viewMatrix;
uniform mat4 uShadow_Texture1_projectionMatrix;
uniform mat4 uShadow_Texture1_viewMatrix;
uniform mat4 uSketchfabLight0_matrix;
uniform mat4 uSketchfabLight1_invMatrix;
uniform mat4 uSketchfabLight1_matrix;
uniform mat4 uSketchfabLight2_matrix;
uniform sampler2D Texture0;
uniform sampler2D Texture12;
uniform sampler2D Texture13;
uniform vec2 uBumpMapSize;
uniform vec2 uDisplacementSize;
uniform vec3 uSketchfabLight0_direction;
uniform vec3 uSketchfabLight1_direction;
uniform vec3 uSketchfabLight2_direction;
uniform vec4 uShadow_Texture0_depthRange;
uniform vec4 uShadow_Texture0_mapSize;
uniform vec4 uShadow_Texture0_renderSize;
uniform vec4 uShadow_Texture1_depthRange;
uniform vec4 uShadow_Texture1_mapSize;
uniform vec4 uShadow_Texture1_renderSize;
uniform vec4 uSketchfabLight0_attenuation;
uniform vec4 uSketchfabLight0_diffuse;
uniform vec4 uSketchfabLight0_ground;
uniform vec4 uSketchfabLight0_position;
uniform vec4 uSketchfabLight1_attenuation;
uniform vec4 uSketchfabLight1_diffuse;
uniform vec4 uSketchfabLight1_ground;
uniform vec4 uSketchfabLight1_position;
uniform vec4 uSketchfabLight2_attenuation;
uniform vec4 uSketchfabLight2_diffuse;
uniform vec4 uSketchfabLight2_ground;
uniform vec4 uSketchfabLight2_position;

varying vec2 vTexCoord0;
varying vec3 vModelNormal;
varying vec3 vModelVertex;
varying vec3 vViewNormal;
varying vec4 vViewVertex;


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



#define LUV

float specularOcclusion(const in int occlude, const in float ao, const in vec3 N, const in vec3 V) {
    if(occlude == 0)
        return 1.0;
    // Yoshiharu Gotanda's specular occlusion approximation:
    // cf http://research.tri-ace.com/Data/cedec2011_RealtimePBR_Implementation_e.pptx pg59
    float d = dot( N, V ) + ao;
    return clamp((d * d) - 1.0 + ao, 0.0, 1.0);
}

float adjustRoughnessNormalMap( const in float roughness, const in vec3 normal ) {
    // Based on The Order : 1886 SIGGRAPH course notes implementation (page 21 notes)
    float normalLen = length(normal);
    if ( normalLen < 1.0) {
        float normalLen2 = normalLen * normalLen;
        float kappa = ( 3.0 * normalLen -  normalLen2 * normalLen )/( 1.0 - normalLen2 );
        // http://www.frostbite.com/2014/11/moving-frostbite-to-pbr/
        // page 91 : they use 0.5/kappa instead
        return min(1.0, sqrt( roughness * roughness + 1.0/kappa ));
    }
    return roughness;
}

float adjustRoughnessGeometry( const in float roughness, const in vec3 normal ) {
    // Geometric Specular Aliasing (slide 43)
    // http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// #ifdef GL_OES_standard_derivatives
//     vec3 vDx = dFdx( normal.xyz );
//     vec3 vDy = dFdy( normal.xyz );
//     return max(roughness, pow( clamp( max( dot( vDx, vDx ), dot( vDy, vDy ) ), 0.0, 1.0 ), 0.333 ));
// #else
    return roughness;
// #endif
}

mat3 environmentTransformPBR( const in mat4 tr ) {
    // TODO trick from animation matrix transpose?
    vec3 x = vec3(tr[0][0], tr[1][0], tr[2][0]);
    vec3 y = vec3(tr[0][1], tr[1][1], tr[2][1]);
    vec3 z = vec3(tr[0][2], tr[1][2], tr[2][2]);
    mat3 m = mat3(x, y, z);
    return m;
}

vec3 evaluateDiffuseSphericalHarmonics( const in vec3 s[9], const in mat3 envTrans, const in vec3 N ) {
    vec3 n = envTrans * N;
    // https://github.com/cedricpinson/envtools/blob/master/Cubemap.cpp#L523
    vec3 result = (s[0]+s[1]*n.y+s[2]*n.z+s[3]*n.x+s[4]*n.y*n.x+s[5]*n.y*n.z+s[6]*(3.0*n.z*n.z-1.0)+s[7]*(n.z*n.x)+s[8]*(n.x*n.x-n.y*n.y));
    return max(result, vec3(0.0));
}

// Frostbite, Lagarde paper p67
// http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr.pdf
float linRoughnessToMipmap( const in float roughnessLinear ) {
    return sqrt(roughnessLinear);
}

vec3 integrateBRDF( const in vec3 specular, const in float r, const in float NoV, const in sampler2D tex ) {
    vec4 rgba = texture2D( tex, vec2(NoV, r) );
    float b = (rgba[3] * 65280.0 + rgba[2] * 255.0);
    float a = (rgba[1] * 65280.0 + rgba[0] * 255.0);
    const float div = 1.0/65535.0;
    return (specular * a + b) * div;
}

// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
// TODO should we use somehow specular f0 ?
vec3 integrateBRDFApprox( const in vec3 specular, const in float roughness, const in float NoV ) {
    const vec4 c0 = vec4( -1, -0.0275, -0.572, 0.022 );
    const vec4 c1 = vec4( 1, 0.0425, 1.04, -0.04 );
    vec4 r = roughness * c0 + c1;
    float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
    vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
    return specular * AB.x + AB.y;
}

vec3 computeIBLDiffuseUE4( const in vec3 normal, const in vec3 albedo, const in mat3 envTrans, const in vec3 sphHarm[9] ) {
    return albedo * evaluateDiffuseSphericalHarmonics( sphHarm, envTrans, normal ) ;
}

// basically whether it's panorama or cubemap we load the adequate glsl
// and we set samplerEnv and prefilterEnvMap
#ifdef CUBEMAP

#define samplerEnv samplerCube
#define prefilterEnvMap prefilterEnvMapCube

#else
#ifdef PANORAMA

#define samplerEnv sampler2D
#define prefilterEnvMap prefilterEnvMapPanorama

#else
// in case there is no environment node ?
vec3 prefilterEnvMap( const in float rLinear, const in vec3 R, const in sampler2D tex, const in vec2 lodRange, const in vec2 size ) {
    return vec3(0.0);
}
#define samplerEnv sampler2D
#endif // PANORAMA

#endif // CUBEMAP

vec3 getSpecularDominantDir( const in vec3 N, const in vec3 R, const in float realRoughness ) {
    float smoothness = 1.0 - realRoughness;
    float lerpFactor = smoothness * ( sqrt( smoothness ) + realRoughness );
    // The result is not normalized as we fetch in a cubemap
    return mix( N, R, lerpFactor );
}

// samplerEnv and prefilterEnvMap are both defined above (cubemap or panorama)
vec3 computeIBLSpecularUE4(
    const in vec3 N,
    const in vec3 V,
    const in float rLinear,
    const in vec3 specular,
    const in mat3 envTrans,
    const in samplerEnv texEnv,
    const in vec2 lodRange,
    const in vec2 size,
    const in vec3 frontNormal
    #ifdef MOBILE
    ){
    #else
    ,const in sampler2D texBRDF ) {
    #endif

    float rough = max( rLinear, 0.0);

    float NoV = dot( N, V );
    vec3 R = normalize( NoV * 2.0 * N - V);
    R = getSpecularDominantDir(N, R, rLinear);
    // could use that, especially if NoV comes from shared preCompSpec
    //vec3 R = reflect(-V, N);

    vec3 prefilteredColor = prefilterEnvMap( rough, envTrans * R, texEnv, lodRange, size );
    // http://marmosetco.tumblr.com/post/81245981087
    // marmoset uses 1.3, we force it to 1.0
    float factor = clamp( 1.0 + dot(R, frontNormal), 0.0, 1.0 );
    prefilteredColor *= factor * factor;
    #ifdef MOBILE
    return prefilteredColor * integrateBRDFApprox( specular, rough, NoV );
    #else
    return prefilteredColor * integrateBRDF( specular, rough, NoV, texBRDF );
    #endif
}


// math.glsl
#define PI 3.141593
#define HALF_PI 1.570796
#define EPS .0000001

// dxtogl.glsl ?
#define saturate(_x) clamp(_x, 0., 1.)
#define atan2(_x, _y) atan(_x, _y)
#define frac(_x) fract(_x)
#define lerp(_x, _y, _t) mix(_x, _y, _t)

// light_common.glsl
// ATTENUATION
float getLightAttenuation(const in float dist, const in vec4 lightAttenuation)
{
    // lightAttenuation(constantEnabled, linearEnabled, quadraticEnabled)
    // TODO find a vector alu instead of 4 scalar
    float constant = lightAttenuation.x;
    float linear = lightAttenuation.y*dist;
    float quadratic = lightAttenuation.z*dist*dist;
    return 1.0 / ( constant + linear + quadratic );
}

// light PBR glsl
#define G1V(dotNV, k) (1./(dotNV*(1.-k)+k))

vec4 LightingFuncPrep(const in vec3 N,
                      const in vec3 V,
                      const in float roughness)
{

    float dotNV = saturate(dot(N,V));
    float alpha = roughness * roughness;
    float k = alpha * .5;
    float visNV = G1V(dotNV,k);

    vec4 prepSpec;

    prepSpec.x = alpha;
    prepSpec.y = alpha * alpha;
    prepSpec.z = k;
    prepSpec.w = visNV;

    return prepSpec;

}

vec3 LightingFuncUsePrepGGX(const vec4 prepSpec,
                            const vec3 N,
                            const vec3 V,
                            const vec3 L,
                            const vec3 F0,
                            const float dotNL)
{

    vec3 H = normalize(V+L);
    float dotNH = saturate(dot(N,H));
    // D
    float alphaSqr = prepSpec.y;
    float denom = dotNH * dotNH *(alphaSqr-1.) + 1.;
    float D = alphaSqr / (PI * denom * denom);
    // F
    float dotLH = saturate(dot(L,H));
    float dotLH5 = pow(1.-dotLH,5.);
    vec3 F = vec3(F0) + (vec3(1.)-F0)*(dotLH5);
    // V
    float visNL = G1V(dotNL, prepSpec.z);
    vec3 specular = D * F * visNL * prepSpec.w;
    return specular;
}

// pure compute Light PBR
vec3 computeLight(const in vec3 lightColor,
                  const in vec3 albedoColor,
                  const in vec3 normal,
                  const in vec3 viewDir,
                  const in vec3 lightDir,
                  const in vec3 specular,
                  const in vec4 prepSpec,
                  const in float dotNL)
{
    vec3 cSpec = LightingFuncUsePrepGGX(prepSpec, normal, viewDir, lightDir, specular, dotNL);
    return lightColor *  dotNL*(albedoColor + cSpec);
}

// STANDARD call for each light
//direction, dist, NDL, attenuation, compute diffuse, compute specular
vec3 computeSpotLightPBRShading(
    const in vec3 normal,
    const in vec3 eyeVector,

    const in vec3 albedo,
    const in vec4 prepSpec,
    const in vec3 specular,

    const in vec3 lightColor,

    const in vec3  lightSpotDirection,
    const in vec4  lightAttenuation,
    const in vec4  lightSpotPosition,
    const in float lightCosSpotCutoff,
    const in float lightSpotBlend,

    const in mat4 lightMatrix,
    const in mat4 lightInvMatrix,

    out vec3 eyeLightPos,
    out vec3 eyeLightDir,
    out bool lighted)
{
    lighted = false;
    eyeLightPos = vec3(lightMatrix * lightSpotPosition);
    eyeLightDir = eyeLightPos - vViewVertex.xyz;
    // compute dist
    float dist = length(eyeLightDir);
    // compute attenuation
    float attenuation = getLightAttenuation(dist, lightAttenuation);
    if (attenuation != 0.0)
    {
        // compute direction
        eyeLightDir = dist > 0.0 ? eyeLightDir / dist :  vec3( 0.0, 1.0, 0.0 );
        if (lightCosSpotCutoff > 0.0)
        {
            //compute lightSpotBlend
            vec3 lightSpotDirectionEye = normalize(mat3(vec3(lightInvMatrix[0]), vec3(lightInvMatrix[1]), vec3(lightInvMatrix[2]))*lightSpotDirection);

            float cosCurAngle = dot(-eyeLightDir, lightSpotDirectionEye);
            float diffAngle = cosCurAngle - lightCosSpotCutoff;
            float spot = 1.0;
            if ( diffAngle < 0.0 ) {
                spot = 0.0;
            } else {
                if ( lightSpotBlend > 0.0 )
                    spot = cosCurAngle * smoothstep(0.0, 1.0, (cosCurAngle - lightCosSpotCutoff) / (lightSpotBlend));
            }

            if (spot > 0.0)
            {
                // compute NdL
                float NdotL = dot(eyeLightDir, normal);
                if (NdotL > 0.0)
                {
                    lighted = true;
                    return spot * attenuation *computeLight(lightColor, albedo, normal, eyeVector, eyeLightDir, specular, prepSpec,  NdotL);
                }
            }
        }
    }
    return vec3(0.0);
}

vec3 computePointLightPBRShading(
    const in vec3 normal,
    const in vec3 eyeVector,

    const in vec3 albedo,
    const in vec4 prepSpec,
    const in vec3 specular,

    const in vec3 lightColor,

    const in vec4 lightPosition,
    const in vec4 lightAttenuation,

    const in mat4 lightMatrix,

    out vec3 eyeLightPos,
    out vec3 eyeLightDir,
    out bool lighted)
{

    eyeLightPos =  vec3(lightMatrix * lightPosition);
    eyeLightDir = eyeLightPos - vViewVertex.xyz;
    float dist = length(eyeLightDir);
    // compute dist
    // compute attenuation
    float attenuation = getLightAttenuation(dist, lightAttenuation);
    if (attenuation != 0.0)
    {
        // compute direction
        eyeLightDir = dist > 0.0 ? eyeLightDir / dist :  vec3( 0.0, 1.0, 0.0 );
        // compute NdL
        float NdotL = dot(eyeLightDir, normal);
        if (NdotL > 0.0)
        {
            lighted = true;
            return  attenuation * computeLight(lightColor, albedo, normal, eyeVector, eyeLightDir,  specular, prepSpec,  NdotL);
        }
    }
    return vec3(0.0);
}

vec3 computeSunLightPBRShading(
    const in vec3 normal,
    const in vec3 eyeVector,

    const in vec3 albedo,
    const in vec4 prepSpec,
    const in vec3 specular,

    const in vec3 lightColor,

    const in vec4 lightPosition,

    const in mat4 lightMatrix,

    out vec3 eyeLightDir,
    out bool lighted)
{

    lighted = false;
    eyeLightDir = normalize( vec3(lightMatrix * lightPosition ) );
    // compute NdL
    float NdotL = dot(eyeLightDir, normal);
    if (NdotL > 0.0)
    {
        lighted = true;
        return computeLight(lightColor, albedo, normal, eyeVector, eyeLightDir, specular, prepSpec,  NdotL);
    }
    return vec3(0.0);
}


//begin shadows


float decodeFloatRGBA( vec4 rgba ) {
    return dot( rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0) );
}

vec4 encodeFloatRGBA( float v ) {
    vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
    enc = fract(enc);
    enc -= enc.yzww * vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
    return enc;
}

vec2 decodeHalfFloatRGBA( vec4 rgba ) {
    return vec2(rgba.x + (rgba.y / 255.0), rgba.z + (rgba.w / 255.0));
}

vec4 encodeHalfFloatRGBA( vec2 v ) {
    const vec2 bias = vec2(1.0 / 255.0, 0.0);
    vec4 enc;
    enc.xy = vec2(v.x, fract(v.x * 255.0));
    enc.xy = enc.xy - (enc.yy * bias);

    enc.zw = vec2(v.y, fract(v.y * 255.0));
    enc.zw = enc.zw - (enc.ww * bias);
    return enc;
}


// end Float codec
float getSingleFloatFromTex(const in sampler2D depths, const in vec2 uv){
#ifndef _FLOATTEX
    return  decodeFloatRGBA(texture2D(depths, uv));
#else
    return texture2D(depths, uv).x;
#endif
}

vec2 getDoubleFloatFromTex(const in sampler2D depths, const in vec2 uv){
#ifndef _FLOATTEX
    return decodeHalfFloatRGBA(texture2D(depths, uv));
#else
    return texture2D(depths, uv).xy;
#endif
}

vec4 getQuadFloatFromTex(const in sampler2D depths, const in vec2 uv){
    return texture2D(depths, uv).xyzw;
}
// end Float codec




// simulation of texture2Dshadow glsl call on HW
// http://codeflow.org/entries/2013/feb/15/soft-shadow-mapping/
float texture2DCompare(const in sampler2D depths, const in vec2 uv, const in float compare, const in vec4 clampDimension){

    float depth = getSingleFloatFromTex(depths, clamp(uv, clampDimension.xy, clampDimension.zw));
    return step(compare, depth);

}

// simulates linear fetch like texture2d shadow
float texture2DShadowLerp(const in sampler2D depths, const in vec4 size, const in vec2 uv, const in float compare, const in vec4 clampDimension){

    vec2 f = fract(uv*size.xy+0.5);
    vec2 centroidUV = floor(uv*size.xy+0.5)*size.zw;

    float lb = texture2DCompare(depths, centroidUV+size.zw*vec2(0.0, 0.0), compare, clampDimension);
    float lt = texture2DCompare(depths, centroidUV+size.zw*vec2(0.0, 1.0), compare, clampDimension);
    float rb = texture2DCompare(depths, centroidUV+size.zw*vec2(1.0, 0.0), compare, clampDimension);
    float rt = texture2DCompare(depths, centroidUV+size.zw*vec2(1.0, 1.0), compare, clampDimension);
    float a = mix(lb, lt, f.y);
    float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;

}


float getShadowPCF(const in sampler2D depths, const in vec4 size, const in vec2 uv, const in float compare, const in vec2 biasPCF, const in vec4 clampDimension)
{

     float res = 0.0;
     res += texture2DShadowLerp(depths, size,   uv + biasPCF, compare, clampDimension);

#if defined(_PCFx1)

#else

    float dx0 = -size.z;
    float dy0 = -size.w;
    float dx1 = size.z;
    float dy1 = size.w;

#define TSF(o1,o2) texture2DShadowLerp(depths, size, uv + vec2(o1, o2) + biasPCF,  compare, clampDimension)

    res += TSF(dx0, dx0);
    res += TSF(dx0, .0);
    res += TSF(dx0, dx1);

#if defined(_PCFx4)

    res /=4.0;

#elif defined(_PCFx9)
    res += TSF(.0, dx0);
    res += TSF(.0, dx1);

    res += TSF(dx1, dx0);
    res += TSF(dx1, .0);
    res += TSF(dx1, dx1);


    res /=9.0;

#elif defined(_PCFx25)

    float dx02 = -2.0*size.z;
    float dy02 = -2.0*size.w;
    float dx2 = 2.0*size.z;
    float dy2 = 2.0*size.w;

    // complete row above
    res += TSF(dx0, dx02);
    res += TSF(dx0, dx2);

    res += TSF(.0, dx02);
    res += TSF(.0, dx2);

    res += TSF(dx1, dx02);
    res += TSF(dx1, dx2);

    // two new col
    res += TSF(dx02, dx02);
    res += TSF(dx02, dx0);
    res += TSF(dx02, .0);
    res += TSF(dx02, dx1);
    res += TSF(dx02, dx2);

    res += TSF(dx2, dx02);
    res += TSF(dx2, dx0);
    res += TSF(dx2, .0);
    res += TSF(dx2, dx1);
    res += TSF(dx2, dx2);


    res/=25.0;

#endif

#undef TSF

#endif
    return res;
}
/////// end Tap


#ifdef _ATLAS_SHADOW

float computeShadow(const in bool lighted,
                    const in sampler2D tex,
                    const in vec4 shadowMapSize,
                    const in vec4 shadowTextureSize,
                    const in mat4 shadowProjectionMatrix,
                    const in mat4 shadowViewMatrix,
                    const in vec4 depthRange,
                    const in vec3 normalWorld,
                    const in vec3 vertexWorld,                    
                    const in float bias
#ifdef _NORMAL_OFFSET
                    ,const in float normalBias
#endif //_NORMAL_OFFSET
                    
    )
#else
    
float computeShadow(const in bool lighted,
                    const in sampler2D tex,
                    const in vec4 shadowTextureSize,
                    const in mat4 shadowProjectionMatrix,
                    const in mat4 shadowViewMatrix,
                    const in vec4 depthRange,
                    const in vec3 normalWorld,
                    const in vec3 vertexWorld,
                    const in float bias
#ifdef _NORMAL_OFFSET
                    ,const in float normalBias
#endif //_NORMAL_OFFSET

   )
    
#endif
{
                        
    // 0 for early out
bool earlyOut = false;

// Calculate shadow amount
float shadow = 1.0;

if(!lighted) {
    shadow = 0.0;
    earlyOut = true;
}

if (depthRange.x == depthRange.y) {
    earlyOut = true;
}

vec4 shadowVertexEye;
vec4 shadowNormalEye;
float shadowReceiverZ = 0.0;
vec4 shadowVertexProjected;
vec2 shadowUV;
float N_Dot_L;

if(!earlyOut) {

    shadowVertexEye =  shadowViewMatrix *  vec4(vertexWorld, 1.0);

    vec3 shadowLightDir = vec3(0.0, 0.0, 1.0); // in shadow view light is camera
    vec4 normalFront = vec4(normalWorld, 0.0);
    shadowNormalEye =  shadowViewMatrix * normalFront;
    N_Dot_L = dot(shadowNormalEye.xyz, shadowLightDir);

    if(!earlyOut) {

#ifdef _NORMAL_OFFSET

        // http://www.dissidentlogic.com/old/images/NormalOffsetShadows/GDC_Poster_NormalOffset.png
        float normalOffsetScale = clamp(1.0  - N_Dot_L, 0.0 , 1.0);

        normalOffsetScale *= abs((shadowVertexEye.z - depthRange.x) * depthRange.w) * max(shadowProjectionMatrix[0][0], shadowProjectionMatrix[1][1]);

        normalOffsetScale *= normalBias*depthRange.w;

        shadowNormalEye =  shadowViewMatrix *  (normalFront * normalOffsetScale);

        shadowVertexProjected = shadowProjectionMatrix * (shadowVertexEye + shadowNormalEye);

#else

        shadowVertexProjected = shadowProjectionMatrix * shadowVertexEye;

#endif

        if (shadowVertexProjected.w < 0.0) {
            earlyOut = true; // notably behind camera
        }

    }




    if(!earlyOut) {

        shadowUV.xy = shadowVertexProjected.xy / shadowVertexProjected.w;
        shadowUV.xy = shadowUV.xy * 0.5 + 0.5;// mad like

        if(any(bvec4 ( shadowUV.x > 1., shadowUV.x < 0., shadowUV.y > 1., shadowUV.y < 0.))) {
            earlyOut = true;// limits of light frustum
        }

        // most precision near 0, make sure we are near 0 and in [0,1]
        shadowReceiverZ = - shadowVertexEye.z;
        shadowReceiverZ =  (shadowReceiverZ - depthRange.x)* depthRange.w;

        if(shadowReceiverZ < 0.0) {
            earlyOut = true; // notably behind camera
        }

    }
}






// pcf pbias to add on offset
vec2 shadowBiasPCF = vec2 (0.);

#ifdef GL_OES_standard_derivatives


//#define _RECEIVERPLANEDEPTHBIAS
#ifdef _RECEIVERPLANEDEPTHBIAS
vec2 biasUV;

vec3 texCoordDY = dFdx(shadowVertexEye.xyz);
vec3 texCoordDX = dFdy(shadowVertexEye.xyz);

biasUV.x = texCoordDY.y * texCoordDX.z - texCoordDX.y * texCoordDY.z;
biasUV.y = texCoordDX.x * texCoordDY.z - texCoordDY.x * texCoordDX.z;
biasUV *= 1.0 / ((texCoordDX.x * texCoordDY.y) - (texCoordDX.y * texCoordDY.x));

// Static depth biasing to make up for incorrect fractional sampling on the shadow map grid
float fractionalSamplingError = dot(vec2(1.0, 1.0) * shadowTextureSize.zw, abs(biasUV));
float receiverDepthBias = min(fractionalSamplingError, 0.01);

shadowBiasPCF.x = biasUV.x;
shadowBiasPCF.y = biasUV.y;


shadowReceiverZ += receiverDepthBias;

#else

shadowBiasPCF.x = clamp(dFdx(shadowReceiverZ)* shadowTextureSize.z, -1.0, 1.0 );
shadowBiasPCF.y = clamp(dFdy(shadowReceiverZ)* shadowTextureSize.w, -1.0, 1.0 );

#endif
#endif


vec4 clampDimension;

#ifdef _ATLAS_SHADOW

shadowUV.xy  = ((shadowUV.xy * shadowMapSize.zw ) + shadowMapSize.xy) / shadowTextureSize.xy;

// clamp uv bias/filters by half pixel to avoid point filter on border
clampDimension.xy = shadowMapSize.xy + vec2(0.5);
clampDimension.zw = (shadowMapSize.xy + shadowMapSize.zw) - vec2(0.5);

clampDimension = clampDimension / (shadowTextureSize.xyxy);


#else

clampDimension = vec4(0.0, 0.0, 1.0, 1.0);

#endif


// now that derivatives is done
// and we don't access any mipmapped/texgrad texture
// we can early out
// see http://teknicool.tumblr.com/post/77263472964/glsl-dynamic-branching-and-texture-samplers
if (earlyOut) {
    // empty statement because of weird gpu intel bug
} else {


// depth bias: fighting shadow acne (depth imprecsion z-fighting)
float shadowBias = 0.0;




// cosTheta is dot( n, l ), clamped between 0 and 1
//float shadowBias = 0.005*tan(acos(N_Dot_L));
// same but 4 cycles instead of 15
shadowBias += 0.05 *  sqrt( 1. -  N_Dot_L*N_Dot_L) / clamp(N_Dot_L, 0.0005,  1.0);

//That makes sure that plane perpendicular to light doesn't flicker due to
//selfshadowing and 1 = dot(Normal, Light) using a min bias
shadowBias = clamp(shadowBias, 0.00005,  2.0*bias);

// shadowZ must be clamped to [0,1]
// otherwise it's not comparable to
// shadow caster depth map
// which is clamped to [0,1]
// Not doing that makes ALL shadowReceiver > 1.0 black
// because they ALL becomes behind any point in Caster depth map
shadowReceiverZ = clamp(shadowReceiverZ, 0., 1. - shadowBias);

shadowReceiverZ -= shadowBias;

// Now computes Shadow

shadow = getShadowPCF(tex, shadowTextureSize, shadowUV, shadowReceiverZ, shadowBiasPCF, clampDimension);

}

return shadow;


}

 




void main() {
// vars

const vec3 vec3White = vec3(1.0); vec3 geoNormal; vec3 nFrontViewNormal; vec3 frontViewNormal; vec3 materialDiffusePBR; float channelMetalnessPBR; const float floatWhite = float(1.0); vec3 materialAlbedo; vec3 channelAlbedoPBR; vec3 tmp_13; vec3 tmp_14; float materialSpecularf0; float channelSpecularF0; float tmp_19 = 0.0; float tmp_20 = 0.08; float materialRoughness; float channelRoughnessPBR; float tmp_24; vec3 materialSpecularPBR; vec3 tmp_26; vec4 prepSpec; vec3 tmp_52; vec3 tmp_53; vec3 eyeVector; float tmp_56 = -1.0; vec3 tmp_57; const vec4 vec4Black = vec4(0.0); bool lighted0; vec3 lightEyeDir0; vec3 nFrontModelNormal; vec3 frontModelNormal; float tmp_76; vec3 lightAndShadowTempOutput; vec3 lightMatAmbientOutput; vec3 tmp_87; bool lighted1; vec3 lightEyePos1; vec3 lightEyeDir1; float tmp_102; vec3 lightAndShadowTempOutput1; vec3 lightMatAmbientOutput1; vec3 tmp_113; bool lighted2; vec3 lightEyePos2; vec3 lightEyeDir2; vec3 lightMatAmbientOutput2; vec3 tmp_129; float channelOpacity; float tmp_131; float tmp_132; float tmp_133; vec4 tmp_135; vec3 tmp_136; vec4 tmp_138;

// end vars

tmp_131 = texture2D(Texture0, vTexCoord0).a;
tmp_132 = uOpacityInvert == 1 ? 1.0 - tmp_131 : tmp_131;
channelOpacity = tmp_132*uOpacityFactor;
tmp_133 = channelOpacity * float(1 - uOpacityAdditive);
if(channelOpacity == 0.0 || (uDrawOpaque == 1 && tmp_133 < 9.9999e-1) || (uDrawOpaque == 0 && tmp_133 >= 9.9999e-1)) discard;
frontViewNormal = gl_FrontFacing ? vViewNormal : -vViewNormal ;
nFrontViewNormal = normalize( frontViewNormal );

geoNormal = nFrontViewNormal.rgb;
tmp_13 = texture2D(Texture0, vTexCoord0).rgb;
tmp_14 = sRGBToLinear( tmp_13 );

channelAlbedoPBR = tmp_14.rgb*uAlbedoPBRFactor;
materialAlbedo = channelAlbedoPBR.rgb;
channelMetalnessPBR = floatWhite*uMetalnessPBRFactor;
materialDiffusePBR = materialAlbedo * (1.0 - channelMetalnessPBR);
channelSpecularF0 = floatWhite*uSpecularF0Factor;
materialSpecularf0 = mix(tmp_19, tmp_20, channelSpecularF0);
channelRoughnessPBR = floatWhite*uRoughnessPBRFactor;
tmp_24 = max(1.e-4, channelRoughnessPBR);
materialRoughness = adjustRoughnessGeometry( tmp_24, nFrontViewNormal );

materialSpecularPBR = mix( vec3(materialSpecularf0), materialAlbedo, channelMetalnessPBR);
tmp_53 = vViewVertex.rgb;
tmp_52 = normalize( tmp_53 );

eyeVector = tmp_52.rgb*tmp_56;
prepSpec = LightingFuncPrep( geoNormal, eyeVector, materialRoughness );

frontModelNormal = gl_FrontFacing ? vModelNormal : -vModelNormal ;
nFrontModelNormal = normalize( frontModelNormal );

tmp_57 = computeSunLightPBRShading( geoNormal, eyeVector, materialDiffusePBR, prepSpec, materialSpecularPBR, uSketchfabLight0_diffuse.rgb, uSketchfabLight0_position, uSketchfabLight0_matrix, lightEyeDir0, lighted0 );

tmp_76 = computeShadow( lighted0, Texture12, uShadow_Texture0_renderSize, uShadow_Texture0_projectionMatrix, uShadow_Texture0_viewMatrix, uShadow_Texture0_depthRange, nFrontModelNormal, vModelVertex, uShadowReceive0_bias );

lightAndShadowTempOutput = tmp_57.rgb*tmp_76;
lightMatAmbientOutput = vec3White.rgb*vec4Black.rgb;
tmp_87 = computeSpotLightPBRShading( geoNormal, eyeVector, materialDiffusePBR, prepSpec, materialSpecularPBR, uSketchfabLight1_diffuse.rgb, uSketchfabLight1_direction, uSketchfabLight1_attenuation, uSketchfabLight1_position, uSketchfabLight1_spotCutOff, uSketchfabLight1_spotBlend, uSketchfabLight1_matrix, uSketchfabLight1_invMatrix, lightEyePos1, lightEyeDir1, lighted1 );

tmp_102 = computeShadow( lighted1, Texture13, uShadow_Texture1_renderSize, uShadow_Texture1_projectionMatrix, uShadow_Texture1_viewMatrix, uShadow_Texture1_depthRange, nFrontModelNormal, vModelVertex, uShadowReceive1_bias );

lightAndShadowTempOutput1 = tmp_87.rgb*tmp_102;
lightMatAmbientOutput1 = vec3White.rgb*vec4Black.rgb;
tmp_113 = computePointLightPBRShading( geoNormal, eyeVector, materialDiffusePBR, prepSpec, materialSpecularPBR, uSketchfabLight2_diffuse.rgb, uSketchfabLight2_position, uSketchfabLight2_attenuation, uSketchfabLight2_matrix, lightEyePos2, lightEyeDir2, lighted2 );

lightMatAmbientOutput2 = vec3White.rgb*vec4Black.rgb;
tmp_26 = lightAndShadowTempOutput.rgb+lightMatAmbientOutput.rgb+lightAndShadowTempOutput1.rgb+lightMatAmbientOutput1.rgb+tmp_113.rgb+lightMatAmbientOutput2.rgb;
tmp_129 = tmp_26.rgb*floatWhite;
tmp_135.rgb = tmp_129.rgb * channelOpacity;
if(uOutputLinear == 1 ) {
tmp_136.rgb= tmp_135.rgb;
} else {
tmp_136.rgb = linearTosRGB( tmp_135.rgb );
}
tmp_138 = encodeRGBM( tmp_136, uRGBMRange );

gl_FragColor = uDrawOpaque == 1 ? tmp_138 : vec4(tmp_136, tmp_133);
}