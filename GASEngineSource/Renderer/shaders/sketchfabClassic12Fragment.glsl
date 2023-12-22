#version 100
#extension GL_OES_standard_derivatives : enable
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME Classic12
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
uniform mat4 uSketchfabLight1_matrix;
uniform mat4 uSketchfabLight2_matrix;
uniform sampler2D Texture0;
uniform sampler2D Texture12;
uniform sampler2D Texture13;
uniform sampler2D Texture1;
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


////////////////
// ATTENUATION
/////////////
float getLightAttenuation(const in float dist, const in vec4 lightAttenuation) {
    // lightAttenuation(constantEnabled, linearEnabled, quadraticEnabled)
    // TODO find a vector alu instead of 4 scalar
    float constant = lightAttenuation.x;
    float linear = lightAttenuation.y*dist;
    float quadratic = lightAttenuation.z*dist*dist;
    return 1.0 / ( constant + linear + quadratic );
}
//
// LIGHTING EQUATION TERMS
///
void specularCookTorrance(const in vec3 n, const in vec3 l, const in vec3 v, const in float hard, const in vec3 materialSpecular, const in vec3 lightSpecular, out vec3 specularContrib) {
    vec3 h = normalize(v + l);
    float nh = dot(n, h);
    float specfac = 0.0;

    if(nh > 0.0) {
        float nv = max( dot(n, v), 0.0 );
        float i = pow(nh, hard);
        i = i / (0.1 + nv);
        specfac = i;
    }
    // ugly way to fake an energy conservation (mainly to avoid super bright stuffs with low glossiness)
    float att = hard > 100.0 ? 1.0 : smoothstep(0.0, 1.0, hard * 0.01);
    specularContrib = specfac*materialSpecular*lightSpecular*att;
}

void lambert(const in float ndl,  const in vec3 materialDiffuse, const in vec3 lightDiffuse, out vec3 diffuseContrib) {
    diffuseContrib = ndl*materialDiffuse*lightDiffuse;
}
////////////////////////
/// Main func
///////////////////////

/// for each light
//direction, dist, NDL, attenuation, compute diffuse, compute specular

vec3 computeSpotLightShading(
    const in vec3 normal,
    const in vec3 eyeVector,

    const in vec3 materialAmbient,
    const in vec3 materialDiffuse,
    const in vec3 materialSpecular,
    const in float materialShininess,

    const in vec3 lightAmbient,
    const in vec3 lightDiffuse,
    const in vec3 lightSpecular,

    const in vec3  lightSpotDirection,
    const in vec4  lightAttenuation,
    const in vec4  lightSpotPosition,
    const in float lightCosSpotCutoff,
    const in float lightSpotBlend,

    const in mat4 lightMatrix,
    const in mat4 lightInvMatrix,

    out vec3 eyeLightPos,
    out vec3 eyeLightDir,
    out bool lighted) {
    lighted = false;
    eyeLightPos = vec3(lightMatrix * lightSpotPosition);
    eyeLightDir = eyeLightPos - vViewVertex.xyz;
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
            } else {
                if ( lightSpotBlend > 0.0 )
                    spot = cosCurAngle * smoothstep(0.0, 1.0, (cosCurAngle - lightCosSpotCutoff) / (lightSpotBlend));
            }

            if (spot > 0.0) {
                // compute NdL
                float NdotL = dot(eyeLightDir, normal);
                if (NdotL > 0.0) {
                    lighted = true;
                    vec3 diffuseContrib;
                    lambert(NdotL, materialDiffuse, lightDiffuse, diffuseContrib);
                    vec3 specularContrib;
                    specularCookTorrance(normal, eyeLightDir, eyeVector, materialShininess, materialSpecular, lightSpecular, specularContrib);
                    return spot * attenuation * (diffuseContrib + specularContrib);
                }
            }
        }
    }
    return vec3(0.0);
}

vec3 computePointLightShading(
    const in vec3 normal,
    const in vec3 eyeVector,

    const in vec3 materialAmbient,
    const in vec3 materialDiffuse,
    const in vec3 materialSpecular,
    const in float materialShininess,

    const in vec3 lightAmbient,
    const in vec3 lightDiffuse,
    const in vec3 lightSpecular,

    const in vec4 lightPosition,
    const in vec4 lightAttenuation,

    const in mat4 lightMatrix,

    out vec3 eyeLightPos,
    out vec3 eyeLightDir,
    out bool lighted) {

    eyeLightPos =  vec3(lightMatrix * lightPosition);
    eyeLightDir = eyeLightPos - vViewVertex.xyz;
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
            vec3 diffuseContrib;
            lambert(NdotL, materialDiffuse, lightDiffuse, diffuseContrib);
            vec3 specularContrib;
            specularCookTorrance(normal, eyeLightDir, eyeVector, materialShininess, materialSpecular, lightSpecular, specularContrib);
            return attenuation * (diffuseContrib + specularContrib);
        }
    }
    return vec3(0.0);
}

vec3 computeSunLightShading(

    const in vec3 normal,
    const in vec3 eyeVector,

    const in vec3 materialAmbient,
    const in vec3 materialDiffuse,
    const in vec3 materialSpecular,
    const in float materialShininess,

    const in vec3 lightAmbient,
    const in vec3 lightDiffuse,
    const in vec3 lightSpecular,

    const in vec4 lightPosition,

    const in mat4 lightMatrix,

    out vec3 eyeLightDir,
    out bool lighted) {

    lighted = false;
    eyeLightDir = normalize( vec3(lightMatrix * lightPosition ) );
    // compute NdL   // compute NdL
    float NdotL = dot(eyeLightDir, normal);
    if (NdotL > 0.0) {
        lighted = true;
        vec3 diffuseContrib;
        lambert(NdotL, materialDiffuse, lightDiffuse, diffuseContrib);
        vec3 specularContrib;
        specularCookTorrance(normal, eyeLightDir, eyeVector, materialShininess, materialSpecular, lightSpecular, specularContrib);
        return (diffuseContrib + specularContrib);
    }
    return vec3(0.0);
}

vec3 computeHemiLightShading(

    const in vec3 normal,
    const in vec3 eyeVector,

    const in vec3 materialDiffuse,
    const in vec3 materialSpecular,
    const in float materialShininess,

    const in vec3 lightDiffuse,
    const in vec3 lightGround,

    const in vec4 lightPosition,

    const in mat4 lightMatrix,

    out vec3 eyeLightDir,
    out bool lighted) {

    lighted = false;

    eyeLightDir = normalize( vec3(lightMatrix * lightPosition ) );
    float NdotL = dot(eyeLightDir, normal);
    float weight = 0.5 * NdotL + 0.5;
    vec3 diffuseContrib = materialDiffuse * mix(lightGround, lightDiffuse, weight);

    // same cook-torrance as above for sky/ground
    float skyWeight = 0.5 * dot(normal, normalize(eyeVector + eyeLightDir)) + 0.5;
    float gndWeight = 0.5 * dot(normal, normalize(eyeVector - eyeLightDir)) + 0.5;
    float skySpec = pow(skyWeight, materialShininess);
    float skyGround = pow(gndWeight, materialShininess);
    float divisor = (0.1 + max( dot(normal, eyeVector), 0.0 ));
    float att = materialShininess > 100.0 ? 1.0 : smoothstep(0.0, 1.0, materialShininess * 0.01);
    vec3 specularContrib = lightDiffuse * materialSpecular * weight * att * (skySpec + skyGround) / divisor;

    return diffuseContrib + specularContrib;
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

const vec3 vec3White = vec3(1.0); vec3 materialDiffuseColor; vec3 channelDiffuseColor; vec3 tmp_5; vec3 tmp_6; const float floatWhite = float(1.0); vec3 channelSpecularColor; const vec2 uvColorAtlas2 = vec2(0.625, 0.5); vec3 tmp_12; const float floatBlack = float(0.0); vec3 geoNormal; vec3 nFrontViewNormal; vec3 frontViewNormal; vec3 tmp_19; vec3 tmp_46; const vec4 vec4Black = vec4(0.0); bool lighted0; vec3 lightEyeDir0; vec3 tmp_61; vec3 tmp_62; vec3 eyeVector; float tmp_65 = -1.0; vec3 nFrontModelNormal; vec3 frontModelNormal; float tmp_70; vec3 lightAndShadowTempOutput; vec3 lightMatAmbientOutput; vec3 tmp_81; bool lighted1; vec3 lightEyeDir1; float tmp_95; vec3 lightAndShadowTempOutput1; vec3 lightMatAmbientOutput1; vec3 tmp_106; bool lighted2; vec3 lightEyeDir2; vec3 lightMatAmbientOutput2; float channelOpacity; float tmp_122; float tmp_123; float tmp_124; vec4 tmp_126; vec3 tmp_127; vec4 tmp_129;

// end vars

tmp_122 = texture2D(Texture0, vTexCoord0).a;
tmp_123 = uOpacityInvert == 1 ? 1.0 - tmp_122 : tmp_122;
channelOpacity = tmp_123*uOpacityFactor;
tmp_124 = channelOpacity * float(1 - uOpacityAdditive);
if(channelOpacity == 0.0 || (uDrawOpaque == 1 && tmp_124 < 9.9999e-1) || (uDrawOpaque == 0 && tmp_124 >= 9.9999e-1)) discard;
tmp_5 = texture2D(Texture0, vTexCoord0).rgb;
tmp_6 = sRGBToLinear( tmp_5 );

channelDiffuseColor = tmp_6.rgb*uDiffuseColorFactor;
materialDiffuseColor = channelDiffuseColor.rgb*floatWhite;
tmp_12 = texture2D(Texture1, uvColorAtlas2).rgb;
channelSpecularColor = tmp_12.rgb*uSpecularColorFactor;
frontViewNormal = gl_FrontFacing ? vViewNormal : -vViewNormal ;
nFrontViewNormal = normalize( frontViewNormal );

geoNormal = nFrontViewNormal.rgb;
tmp_62 = vViewVertex.rgb;
tmp_61 = normalize( tmp_62 );

eyeVector = tmp_61.rgb*tmp_65;
frontModelNormal = gl_FrontFacing ? vModelNormal : -vModelNormal ;
nFrontModelNormal = normalize( frontModelNormal );

tmp_46 = computeSunLightShading( geoNormal, eyeVector, vec3White, materialDiffuseColor, channelSpecularColor, floatBlack, vec4Black.rgb, uSketchfabLight0_diffuse.rgb, uSketchfabLight0_diffuse.rgb, uSketchfabLight0_position, uSketchfabLight0_matrix, lightEyeDir0, lighted0 );

tmp_70 = computeShadow( lighted0, Texture12, uShadow_Texture0_renderSize, uShadow_Texture0_projectionMatrix, uShadow_Texture0_viewMatrix, uShadow_Texture0_depthRange, nFrontModelNormal, vModelVertex, uShadowReceive0_bias );

lightAndShadowTempOutput = tmp_46.rgb*tmp_70;
lightMatAmbientOutput = vec3White.rgb*vec4Black.rgb;
tmp_81 = computeSunLightShading( geoNormal, eyeVector, vec3White, materialDiffuseColor, channelSpecularColor, floatBlack, vec4Black.rgb, uSketchfabLight1_diffuse.rgb, uSketchfabLight1_diffuse.rgb, uSketchfabLight1_position, uSketchfabLight1_matrix, lightEyeDir1, lighted1 );

tmp_95 = computeShadow( lighted1, Texture13, uShadow_Texture1_renderSize, uShadow_Texture1_projectionMatrix, uShadow_Texture1_viewMatrix, uShadow_Texture1_depthRange, nFrontModelNormal, vModelVertex, uShadowReceive1_bias );

lightAndShadowTempOutput1 = tmp_81.rgb*tmp_95;
lightMatAmbientOutput1 = vec3White.rgb*vec4Black.rgb;
tmp_106 = computeHemiLightShading( geoNormal, eyeVector, materialDiffuseColor, channelSpecularColor, floatBlack, uSketchfabLight2_diffuse.rgb, uSketchfabLight2_ground.rgb, uSketchfabLight2_position, uSketchfabLight2_matrix, lightEyeDir2, lighted2 );

lightMatAmbientOutput2 = vec3White.rgb*vec4Black.rgb;
tmp_19 = lightAndShadowTempOutput.rgb+lightMatAmbientOutput.rgb+lightAndShadowTempOutput1.rgb+lightMatAmbientOutput1.rgb+tmp_106.rgb+lightMatAmbientOutput2.rgb;
tmp_126.rgb = tmp_19.rgb * channelOpacity;
if(uOutputLinear == 1 ) {
tmp_127.rgb= tmp_126.rgb;
} else {
tmp_127.rgb = linearTosRGB( tmp_126.rgb );
}
tmp_129 = encodeRGBM( tmp_127, uRGBMRange );

gl_FragColor = uDrawOpaque == 1 ? tmp_129 : vec4(tmp_127, tmp_124);
}