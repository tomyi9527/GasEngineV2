#version 100
#extension GL_EXT_shader_texture_lod : require
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif

#define CUBEMAP
#define SHADER_NAME PBR15


uniform float uAlbedoPBRFactor;
uniform float uEnvironmentExposure;
uniform float uMetalnessPBRFactor;
uniform float uRGBMRange;
uniform float uRoughnessPBRFactor;
uniform float uSpecularF0Factor;
uniform int uOutputLinear;
uniform mat4 uEnvironmentTransform;
uniform sampler2D sIntegrateBRDF;
uniform sampler2D sTextureAlbedoPBR;
uniform sampler2D sTextureMetalnessPBR;
uniform sampler2D sTextureRoughnessPBR;
uniform sampler2D sTextureSpecularF0;
uniform samplerCube sSpecularPBR;
uniform vec2 uTextureEnvironmentSpecularPBRLodRange;
uniform vec2 uTextureEnvironmentSpecularPBRTextureSize;
uniform vec3 uDiffuseSPH[9];

varying vec2 vTexCoord0;
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

vec3 textureRGB(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv.xy ).rgb;
}

vec4 textureRGBA(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv.xy ).rgba;
}

float textureIntensity(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv).r;
}

float textureAlpha(const in sampler2D texture, const in vec2 uv) {
    return texture2D(texture, uv.xy ).a;
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
    vec4 rgba = textureCubeLodEXT( texture, dir, lod );
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

    float NoV = clamp( dot( N, V ), 0.0, 1.0 );
    vec3 R = normalize( NoV * 2.0 * N - V);
    R = getSpecularDominantDir(N, R, rLinear);
    // could use that, especially if NoV comes from shared preCompSpec
    //vec3 R = reflect(-V, N);

    vec3 prefilteredColor = prefilterEnvMap( rough, envTrans * R, texEnv, lodRange, size );
    // http://marmosetco.tumblr.com/post/81245981087
    // TODO we set a min value (10%) to avoid pure blackness (in case of pure metal)
    float factor = clamp( 1.0 + 1.3 * dot(R, frontNormal), 0.1, 1.0 );
    prefilteredColor *= factor * factor;
    #ifdef MOBILE
    return prefilteredColor * integrateBRDFApprox( specular, rough, NoV );
    #else
    return prefilteredColor * integrateBRDF( specular, rough, NoV, texBRDF );
    #endif
}






void main() {
// vars

mat3 tmp_0; vec3 tmp_3; vec3 geoNormal; vec3 normal; vec3 frontNormal; vec3 materialDiffusePBR; float tmp_11; float channelMetalnessPBR; vec3 materialAlbedo; vec3 tmp_16; vec3 tmp_17; vec3 channelAlbedoPBR; vec3 tmp_20; const float floatWhite = float(1.0); vec3 tmp_23; vec3 tmp_24; vec3 eyeVector; float tmp_27 = -1.0; float materialRoughness; float tmp_30; float channelRoughnessPBR; float tmp_33; vec3 materialSpecularPBR; float materialSpecularf0; float tmp_37; float channelSpecularF0; float tmp_40 = 0.0; float tmp_41 = 0.08; vec3 tmp_45; vec3 tmp_46; float materialAO = 1.0; vec3 tmp_48; vec3 tmp_49; vec3 tmp_51; vec3 tmp_52;

// end vars

frontNormal = gl_FrontFacing ? vViewNormal : -vViewNormal ;
normal = normalize( frontNormal );

geoNormal = normal.rgb;
tmp_16 = textureRGB( sTextureAlbedoPBR, vTexCoord0.xy );

tmp_17 = sRGBToLinear( tmp_16 );

channelAlbedoPBR = tmp_17.rgb*uAlbedoPBRFactor;
materialAlbedo = channelAlbedoPBR.rgb;
tmp_11 = textureIntensity( sTextureMetalnessPBR, vTexCoord0.xy );

channelMetalnessPBR = tmp_11*uMetalnessPBRFactor;
materialDiffusePBR = materialAlbedo * (1.0 - channelMetalnessPBR);
tmp_0 = environmentTransformPBR( uEnvironmentTransform );

tmp_3 = computeIBLDiffuseUE4( geoNormal, materialDiffusePBR, tmp_0, uDiffuseSPH );

tmp_20 = tmp_3.rgb*floatWhite;
tmp_24 = vViewVertex.rgb;
tmp_23 = normalize( tmp_24 );

eyeVector = tmp_23.rgb*tmp_27;
tmp_30 = textureIntensity( sTextureRoughnessPBR, vTexCoord0.xy );

channelRoughnessPBR = tmp_30*uRoughnessPBRFactor;
tmp_33 = max(1.e-4, channelRoughnessPBR);
materialRoughness = adjustRoughnessGeometry( tmp_33, normal );

tmp_37 = textureIntensity( sTextureSpecularF0, vTexCoord0.xy );

channelSpecularF0 = tmp_37*uSpecularF0Factor;
materialSpecularf0 = mix(tmp_40, tmp_41, channelSpecularF0);
materialSpecularPBR = mix( vec3(materialSpecularf0), materialAlbedo, channelMetalnessPBR);
tmp_45 = computeIBLSpecularUE4( geoNormal, eyeVector, materialRoughness, materialSpecularPBR, tmp_0, sSpecularPBR, uTextureEnvironmentSpecularPBRLodRange, uTextureEnvironmentSpecularPBRTextureSize, normal, sIntegrateBRDF );

tmp_46 = tmp_45.rgb*materialAO;
tmp_48 = tmp_20.rgb+tmp_46.rgb;
tmp_49 = tmp_48.rgb*uEnvironmentExposure;
tmp_51 = tmp_49.rgb*floatWhite;
if(uOutputLinear == 1 ) {
tmp_52= tmp_51;
} else {
tmp_52 = linearTosRGB( tmp_51 );
}
gl_FragColor = encodeRGBM( tmp_52, uRGBMRange );

}