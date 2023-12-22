#version 100
#extension GL_EXT_shader_texture_lod : require
#extension GL_OES_standard_derivatives : enable
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif

#define CUBEMAP
#define SHADER_NAME PBR(02_Default_0)
#define _PCFx1


uniform float uAlbedoPBRFactor;
uniform float uEnvironmentExposure;
uniform float uGlossinessPBRFactor;
uniform float uMetalnessPBRFactor;
uniform float uNormalMapFactor;
uniform float uRGBMRange;
uniform float uShadowReceive3_bias;
uniform float uSpecularF0Factor;
uniform int uNormalMapFlipY;
uniform int uOutputLinear;
uniform mat4 uEnvironmentTransform;
uniform mat4 uShadow_Texture3_projectionMatrix;
uniform mat4 uShadow_Texture3_viewMatrix;
uniform mat4 uSketchfabLight3_matrix;
uniform sampler2D Texture0;
uniform sampler2D Texture15;
uniform sampler2D Texture1;
uniform sampler2D sIntegrateBRDF;
uniform samplerCube sSpecularPBR;
uniform vec2 uTextureEnvironmentSpecularPBRLodRange;
uniform vec2 uTextureEnvironmentSpecularPBRTextureSize;
uniform vec3 uDiffuseSPH[9];
uniform vec4 uShadow_Texture3_depthRange;
uniform vec4 uShadow_Texture3_renderSize;
uniform vec4 uSketchfabLight3_diffuse;
uniform vec4 uSketchfabLight3_position;

varying vec2 vTexCoord0;
varying vec3 vModelNormal;
varying vec3 vModelVertex;
varying vec3 vViewNormal;
varying vec4 vViewTangent;
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

float adjustSpecular( const in float specular, const in vec3 normal ) {
    // Based on The Order : 1886 SIGGRAPH course notes implementation (page 21 notes)
    float normalLen = length(normal);
    if ( normalLen < 1.0) {
        float normalLen2 = normalLen * normalLen;
        float kappa = ( 3.0 * normalLen -  normalLen2 * normalLen )/( 1.0 - normalLen2 );
        // http://www.frostbite.com/2014/11/moving-frostbite-to-pbr/
        // page 91 : they use 0.5/kappa instead
        return 1.0-min(1.0, sqrt( (1.0-specular) * (1.0-specular) + 1.0/kappa ));
    }
    return specular;
}

vec3 mtexNspaceTangent(const in vec4 tangent, const in vec3 normal, const in vec3 texnormal) {
    vec3 tang = vec3(0.0,1.0,0.0);
    float l = length(tangent.xyz);
    if (l != 0.0) {
        //normalize reusing length computations
        // tang =  normalize(tangent.xyz);
        tang =  tangent.xyz / l;
    }
    vec3 B = tangent.w * normalize(cross(normal, tang));
    return normalize( texnormal.x*tang + texnormal.y*B + texnormal.z*normal);
}

vec2 normalMatcap(const in vec3 normal, const in vec3 nm_z) {
    vec3 nm_x = vec3(-nm_z.z, 0.0, nm_z.x);
    vec3 nm_y = cross(nm_x, nm_z);
    return vec2(dot(normal.xz, -nm_x.xz), dot(normal, nm_y)) * vec2(0.5) + vec2(0.5);
}

vec3 rgbToNormal(const in vec3 texel, const in int flipNormalY) {
    vec3 rgb = texel * vec3(2.0) + vec3(-1.0); // MADD vec form
    rgb[1] = flipNormalY == 1 ? -rgb[1] : rgb[1];
    return rgb;
}

vec3 bumpMap(const in vec4 tangent, const in vec3 normal, const in vec2 gradient) {
    vec3 outnormal;
    float l = length(tangent.xyz);
    if (l != 0.0) {
        //normalize reusing length computations
        // vec3 tang =  normalize(tangent.xyz);
        vec3 tang =  tangent.xyz / l;
        vec3 binormal = tangent.w * normalize(cross(normal, tang));
        outnormal = normal + gradient.x * tang + gradient.y * binormal;
    }
    else {
       outnormal = vec3(normal.x + gradient.x, normal.y + gradient.y, normal.z);
    }
    return normalize(outnormal);
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
// has to be before anything else, just after glsl language version
// it's require. enable just allow
vec3 textureCubemapLod(const in samplerCube texture, const in vec3 dir, const in float lod ) {
    vec4 rgba = textureLod( texture, dir, lod );
#ifdef FLOAT
    return rgba.rgb;
#endif
#ifdef RGBM
    return RGBMToRGB( rgba );
#endif
#ifdef LUV
    return LUVToRGB( rgba );
#endif
}

vec3 textureCubeLodEXTFixed(const in samplerCube texture, const in vec2 size, const in vec3 direction, const in float lodInput, const in float maxLod ) {
    vec3 dir = direction;
    float lod = min( maxLod, lodInput );

    // http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
    float scale = 1.0 - exp2(lod) / size.x;
    vec3 absDir = abs(dir);
    float M = max(max(absDir.x, absDir.y), absDir.z);

    if (absDir.x != M) dir.x *= scale;
    if (absDir.y != M) dir.y *= scale;
    if (absDir.z != M) dir.z *= scale;

    return textureCubemapLod( texture, dir, lod );
}

vec3 prefilterEnvMapCube( const in float rLinear, const in vec3 R, const in samplerCube tex, const in vec2 lodRange, const in vec2 size ){
    float lod = linRoughnessToMipmap(rLinear) * lodRange[1]; //( uEnvironmentMaxLod - 1.0 );
    return textureCubeLodEXTFixed( tex, size, R, lod, lodRange[0] );
}

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
float getLightAttenuation(const in float dist, const in vec4 lightAttenuation) {
    // lightAttenuation(constantEnabled, linearEnabled, quadraticEnabled)
    // TODO find a vector alu instead of 4 scalar
    float constant = lightAttenuation.x;
    float linear = lightAttenuation.y*dist;
    float quadratic = lightAttenuation.z*dist*dist;
    return 1.0 / ( constant + linear + quadratic );
}

// light PBR glsl
#define G1V(dotNV, k) (1./(dotNV*(1.-k)+k))

vec4 LightingFuncPrep(const in vec3 N, const in vec3 V, const in float roughness) {

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

vec3 LightingFuncUsePrepGGX(const vec4 prepSpec, const vec3 N, const vec3 V, const vec3 L, const vec3 F0, const float dotNL) {

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
                  const in vec3 albedo,
                  const in vec3 normal,
                  const in vec3 viewDir,
                  const in vec3 lightDir,
                  const in vec3 specular,
                  const in vec4 prepSpec,
                  const in float dotNL) {

    vec3 cSpec = LightingFuncUsePrepGGX(prepSpec, normal, viewDir, lightDir, specular, dotNL);
    return lightColor * dotNL * (albedo + cSpec);
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
    const in vec3 lightSpotDirection,
    const in vec4 lightAttenuation,
    const in vec4 lightSpotPosition,
    const in float lightCosSpotCutoff,
    const in float lightSpotBlend,
    const in mat4 lightMatrix,
    const in mat4 lightInvMatrix,

    out bool lighted) {

    lighted = false;
    vec3 eyeLightPos = vec3(lightMatrix * lightSpotPosition);
    vec3 eyeLightDir = eyeLightPos - vViewVertex.xyz;
    // compute dist
    float dist = length(eyeLightDir);
    // compute attenuation
    float attenuation = getLightAttenuation(dist, lightAttenuation);
    if (attenuation != 0.0) {
        // compute direction
        eyeLightDir = dist > 0.0 ? eyeLightDir / dist :  vec3( 0.0, 1.0, 0.0 );

        if (lightCosSpotCutoff > 0.0) {
            //compute lightSpotBlend
            vec3 lightSpotDirectionEye = normalize(mat3(vec3(lightInvMatrix[0]), vec3(lightInvMatrix[1]), vec3(lightInvMatrix[2]))*lightSpotDirection);

            float cosCurAngle = dot(-eyeLightDir, lightSpotDirectionEye);
            float diffAngle = cosCurAngle - lightCosSpotCutoff;
            float spot = 1.0;
            if ( diffAngle < 0.0 ) {
                spot = 0.0;
            } else if ( lightSpotBlend > 0.0 ) {
                spot = cosCurAngle * smoothstep(0.0, 1.0, (cosCurAngle - lightCosSpotCutoff) / (lightSpotBlend));
            }

            if (spot > 0.0) {
                // compute NdL
                float NdotL = dot(eyeLightDir, normal);
                if (NdotL > 0.0) {
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

    out bool lighted) {

    lighted = false;
    vec3 eyeLightPos =  vec3(lightMatrix * lightPosition);
    vec3 eyeLightDir = eyeLightPos - vViewVertex.xyz;
    float dist = length(eyeLightDir);
    // compute dist
    // compute attenuation
    float attenuation = getLightAttenuation(dist, lightAttenuation);
    if (attenuation != 0.0) {
        // compute direction
        eyeLightDir = dist > 0.0 ? eyeLightDir / dist :  vec3( 0.0, 1.0, 0.0 );
        // compute NdL
        float NdotL = dot(eyeLightDir, normal);
        if (NdotL > 0.0) {
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

    out bool lighted) {

    lighted = false;
    vec3 eyeLightDir = normalize( vec3(lightMatrix * lightPosition ) );
    // compute NdL
    float NdotL = dot(eyeLightDir, normal);
    if (NdotL > 0.0) {
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

vec3 geoNormal; vec3 channelNormalMap; vec3 tmp_4; vec3 nFrontViewNormal; vec3 frontViewNormal; vec3 normalMap; vec3 tmp_11; vec3 tmp_12; vec4 frontViewTangent; vec3 tmp_15; vec3 tmp_16; vec3 eyeVector; float tmp_19 = -1.0; vec3 materialDiffusePBR; float channelMetalnessPBR; const float floatWhite = float(1.0); vec3 materialAlbedo; vec3 channelAlbedoPBR; const vec2 uvColorAtlas0 = vec2(0.125, 0.5); vec3 tmp_28; vec4 prepSpec; float materialRoughness; float channelGlossinessPBR; float tmp_34; float tmp_35; vec3 materialSpecularPBR; float materialSpecularf0; float channelSpecularF0; float tmp_40 = 0.0; float tmp_41 = 0.08; vec3 tmp_45; bool tmp_46; vec3 nFrontModelNormal; vec3 frontModelNormal; float tmp_57; vec3 tmp_58; mat3 tmp_59; vec3 tmp_61; vec3 tmp_63; vec3 tmp_64; vec3 tmp_69; vec3 tmp_70; vec3 tmp_71; vec3 tmp_73; vec3 tmp_74; vec3 tmp_75;

// end vars

frontViewTangent = gl_FrontFacing ? vViewTangent : -vViewTangent ;
frontViewNormal = gl_FrontFacing ? vViewNormal : -vViewNormal ;
nFrontViewNormal = normalize( frontViewNormal );

tmp_4 = texture2D(Texture1, vTexCoord0).rgb;
channelNormalMap = tmp_4.rgb;
normalMap = rgbToNormal( channelNormalMap, uNormalMapFlipY );

tmp_11.xy = uNormalMapFactor * normalMap.xy;
tmp_11.z = normalMap.z;
tmp_12 = mtexNspaceTangent( frontViewTangent, nFrontViewNormal, tmp_11 );

geoNormal = tmp_12.rgb;
tmp_16 = vViewVertex.rgb;
tmp_15 = normalize( tmp_16 );

eyeVector = tmp_15.rgb*tmp_19;
tmp_28 = texture2D(Texture0, uvColorAtlas0).rgb;
channelAlbedoPBR = tmp_28.rgb*uAlbedoPBRFactor;
materialAlbedo = channelAlbedoPBR.rgb;
channelMetalnessPBR = floatWhite*uMetalnessPBRFactor;
materialDiffusePBR = materialAlbedo * (1.0 - channelMetalnessPBR);
channelGlossinessPBR = floatWhite*uGlossinessPBRFactor;
tmp_34 = 1.0 - channelGlossinessPBR;
tmp_35 = max(1.e-4, tmp_34);
materialRoughness = adjustRoughnessGeometry( tmp_35, nFrontViewNormal );

prepSpec = LightingFuncPrep( geoNormal, eyeVector, materialRoughness );

channelSpecularF0 = floatWhite*uSpecularF0Factor;
materialSpecularf0 = mix(tmp_40, tmp_41, channelSpecularF0);
materialSpecularPBR = mix( vec3(materialSpecularf0), materialAlbedo, channelMetalnessPBR);
tmp_45 = computeSunLightPBRShading( geoNormal, eyeVector, materialDiffusePBR, prepSpec, materialSpecularPBR, uSketchfabLight3_diffuse.rgb, uSketchfabLight3_position, uSketchfabLight3_matrix, tmp_46 );

frontModelNormal = gl_FrontFacing ? vModelNormal : -vModelNormal ;
nFrontModelNormal = normalize( frontModelNormal );

tmp_57 = computeShadow( tmp_46, Texture15, uShadow_Texture3_renderSize, uShadow_Texture3_projectionMatrix, uShadow_Texture3_viewMatrix, uShadow_Texture3_depthRange, nFrontModelNormal, vModelVertex, uShadowReceive3_bias );

tmp_58 = tmp_45.rgb*tmp_57;
tmp_59 = environmentTransformPBR( uEnvironmentTransform );

tmp_63 = computeIBLDiffuseUE4( geoNormal, materialDiffusePBR, tmp_59, uDiffuseSPH );

tmp_64 = tmp_63.rgb*floatWhite;
tmp_69 = computeIBLSpecularUE4( geoNormal, eyeVector, materialRoughness, materialSpecularPBR, tmp_59, sSpecularPBR, uTextureEnvironmentSpecularPBRLodRange, uTextureEnvironmentSpecularPBRTextureSize, nFrontViewNormal, sIntegrateBRDF );

tmp_70 = tmp_69.rgb*floatWhite;
tmp_71 = tmp_64.rgb+tmp_70.rgb;
tmp_61 = tmp_71.rgb*uEnvironmentExposure;
tmp_73 = tmp_58.rgb+tmp_61.rgb;
tmp_74 = tmp_73.rgb*floatWhite;
if(uOutputLinear == 1 ) {
tmp_75= tmp_74;
} else {
tmp_75 = linearTosRGB( tmp_74 );
}
gl_FragColor = encodeRGBM( tmp_75, uRGBMRange );

}