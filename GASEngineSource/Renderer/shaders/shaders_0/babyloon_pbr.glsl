#define REFLECTION
#define NORMAL
#define NUM_BONE_INFLUENCERS 0
#define BonesPerMesh 0
#define REFLECTIONMAP_3D
#define REFLECTIONMAP_CUBIC
#define CAMERATONEMAP
#define CAMERACONTRAST
#define USESPHERICALFROMREFLECTIONMAP
#define LODBASEDMICROSFURACE
#define USEPHYSICALLIGHTFALLOFF
#define RADIANCEOVERALPHA

#ifdef BUMP
#extension GL_OES_standard_derivatives : enable
#endif
#ifdef LODBASEDMICROSFURACE
#extension GL_EXT_shader_texture_lod : enable
#endif
#ifdef LOGARITHMICDEPTH
#extension GL_EXT_frag_depth : enable
#endif
precision highp float;
uniform vec3 vEyePosition;
uniform vec3 vAmbientColor;
uniform vec3 vReflectionColor;
uniform vec4 vAlbedoColor;

uniform vec4 vLightingIntensity;
uniform vec4 vCameraInfos;
#ifdef OVERLOADEDVALUES
uniform vec4 vOverloadedIntensity;
uniform vec3 vOverloadedAmbient;
uniform vec3 vOverloadedAlbedo;
uniform vec3 vOverloadedReflectivity;
uniform vec3 vOverloadedEmissive;
uniform vec3 vOverloadedReflection;
uniform vec3 vOverloadedMicroSurface;
#endif
#ifdef OVERLOADEDSHADOWVALUES
uniform vec4 vOverloadedShadowIntensity;
#endif
#if defined(REFLECTION) || defined(REFRACTION)
uniform vec2 vMicrosurfaceTextureLods;
#endif
uniform vec4 vReflectivityColor;
uniform vec3 vEmissiveColor;

varying vec3 vPositionW;
#ifdef NORMAL
varying vec3 vNormalW;
#endif
#ifdef VERTEXCOLOR
varying vec4 vColor;
#endif

#ifdef LIGHT0
uniform vec4 vLightData0;
uniform vec4 vLightDiffuse0;
#ifdef SPECULARTERM
uniform vec3 vLightSpecular0;
#endif
#ifdef SHADOW0
#if defined(SPOTLIGHT0) || defined(DIRLIGHT0)
varying vec4 vPositionFromLight0;
uniform sampler2D shadowSampler0;
#else
uniform samplerCube shadowSampler0;
#endif
uniform vec3 shadowsInfo0;
#endif
#ifdef SPOTLIGHT0
uniform vec4 vLightDirection0;
#endif
#ifdef HEMILIGHT0
uniform vec3 vLightGround0;
#endif
#endif
#ifdef LIGHT1
uniform vec4 vLightData1;
uniform vec4 vLightDiffuse1;
#ifdef SPECULARTERM
uniform vec3 vLightSpecular1;
#endif
#ifdef SHADOW1
#if defined(SPOTLIGHT1) || defined(DIRLIGHT1)
varying vec4 vPositionFromLight1;
uniform sampler2D shadowSampler1;
#else
uniform samplerCube shadowSampler1;
#endif
uniform vec3 shadowsInfo1;
#endif
#ifdef SPOTLIGHT1
uniform vec4 vLightDirection1;
#endif
#ifdef HEMILIGHT1
uniform vec3 vLightGround1;
#endif
#endif
#ifdef LIGHT2
uniform vec4 vLightData2;
uniform vec4 vLightDiffuse2;
#ifdef SPECULARTERM
uniform vec3 vLightSpecular2;
#endif
#ifdef SHADOW2
#if defined(SPOTLIGHT2) || defined(DIRLIGHT2)
varying vec4 vPositionFromLight2;
uniform sampler2D shadowSampler2;
#else
uniform samplerCube shadowSampler2;
#endif
uniform vec3 shadowsInfo2;
#endif
#ifdef SPOTLIGHT2
uniform vec4 vLightDirection2;
#endif
#ifdef HEMILIGHT2
uniform vec3 vLightGround2;
#endif
#endif
#ifdef LIGHT3
uniform vec4 vLightData3;
uniform vec4 vLightDiffuse3;
#ifdef SPECULARTERM
uniform vec3 vLightSpecular3;
#endif
#ifdef SHADOW3
#if defined(SPOTLIGHT3) || defined(DIRLIGHT3)
varying vec4 vPositionFromLight3;
uniform sampler2D shadowSampler3;
#else
uniform samplerCube shadowSampler3;
#endif
uniform vec3 shadowsInfo3;
#endif
#ifdef SPOTLIGHT3
uniform vec4 vLightDirection3;
#endif
#ifdef HEMILIGHT3
uniform vec3 vLightGround3;
#endif
#endif
#ifdef LIGHT4
uniform vec4 vLightData4;
uniform vec4 vLightDiffuse4;
#ifdef SPECULARTERM
uniform vec3 vLightSpecular4;
#endif
#ifdef SHADOW4
#if defined(SPOTLIGHT4) || defined(DIRLIGHT4)
varying vec4 vPositionFromLight4;
uniform sampler2D shadowSampler4;
#else
uniform samplerCube shadowSampler4;
#endif
uniform vec3 shadowsInfo4;
#endif
#ifdef SPOTLIGHT4
uniform vec4 vLightDirection4;
#endif
#ifdef HEMILIGHT4
uniform vec3 vLightGround4;
#endif
#endif


#ifdef ALBEDO
varying vec2 vAlbedoUV;
uniform sampler2D albedoSampler;
uniform vec2 vAlbedoInfos;
#endif
#ifdef AMBIENT
varying vec2 vAmbientUV;
uniform sampler2D ambientSampler;
uniform vec3 vAmbientInfos;
#endif
#ifdef OPACITY 
varying vec2 vOpacityUV;
uniform sampler2D opacitySampler;
uniform vec2 vOpacityInfos;
#endif
#ifdef EMISSIVE
varying vec2 vEmissiveUV;
uniform vec2 vEmissiveInfos;
uniform sampler2D emissiveSampler;
#endif
#ifdef LIGHTMAP
varying vec2 vLightmapUV;
uniform vec2 vLightmapInfos;
uniform sampler2D lightmapSampler;
#endif
#if defined(REFLECTIVITY) || defined(METALLICWORKFLOW) 
varying vec2 vReflectivityUV;
uniform vec2 vReflectivityInfos;
uniform sampler2D reflectivitySampler;
#endif

#ifdef FRESNEL
float computeFresnelTerm(vec3 viewDirection,vec3 worldNormal,float bias,float power)
{
float fresnelTerm=pow(bias+abs(dot(viewDirection,worldNormal)),power);
return clamp(fresnelTerm,0.,1.);
}
#endif
#ifdef OPACITYFRESNEL
uniform vec4 opacityParts;
#endif
#ifdef EMISSIVEFRESNEL
uniform vec4 emissiveLeftColor;
uniform vec4 emissiveRightColor;
#endif

#if defined(REFLECTIONMAP_SPHERICAL) || defined(REFLECTIONMAP_PROJECTION) || defined(REFRACTION)
uniform mat4 view;
#endif

#ifdef REFRACTION
uniform vec4 vRefractionInfos;
#ifdef REFRACTIONMAP_3D
uniform samplerCube refractionCubeSampler;
#else
uniform sampler2D refraction2DSampler;
uniform mat4 refractionMatrix;
#endif
#endif

#ifdef REFLECTION
uniform vec2 vReflectionInfos;
#ifdef REFLECTIONMAP_3D
uniform samplerCube reflectionCubeSampler;
#else
uniform sampler2D reflection2DSampler;
#endif
#ifdef REFLECTIONMAP_SKYBOX
varying vec3 vPositionUVW;
#else
#ifdef REFLECTIONMAP_EQUIRECTANGULAR_FIXED
varying vec3 vDirectionW;
#endif
#if defined(REFLECTIONMAP_PLANAR) || defined(REFLECTIONMAP_CUBIC) || defined(REFLECTIONMAP_PROJECTION)
uniform mat4 reflectionMatrix;
#endif
#endif
vec3 computeReflectionCoords(vec4 worldPos,vec3 worldNormal)
{
#ifdef REFLECTIONMAP_EQUIRECTANGULAR_FIXED
vec3 direction=normalize(vDirectionW);
float t=clamp(direction.y*-0.5+0.5,0.,1.0);
float s=atan(direction.z,direction.x)*RECIPROCAL_PI2+0.5;
return vec3(s,t,0);
#endif
#ifdef REFLECTIONMAP_EQUIRECTANGULAR
vec3 cameraToVertex=normalize(worldPos.xyz-vEyePosition);
vec3 r=reflect(cameraToVertex,worldNormal);
float t=clamp(r.y*-0.5+0.5,0.,1.0);
float s=atan(r.z,r.x)*RECIPROCAL_PI2+0.5;
return vec3(s,t,0);
#endif
#ifdef REFLECTIONMAP_SPHERICAL
vec3 viewDir=normalize(vec3(view*worldPos));
vec3 viewNormal=normalize(vec3(view*vec4(worldNormal,0.0)));
vec3 r=reflect(viewDir,viewNormal);
r.z=r.z-1.0;
float m=2.0*length(r);
return vec3(r.x/m+0.5,1.0-r.y/m-0.5,0);
#endif
#ifdef REFLECTIONMAP_PLANAR
vec3 viewDir=worldPos.xyz-vEyePosition;
vec3 coords=normalize(reflect(viewDir,worldNormal));
return vec3(reflectionMatrix*vec4(coords,1));
#endif
#ifdef REFLECTIONMAP_CUBIC
vec3 viewDir=worldPos.xyz-vEyePosition;
vec3 coords=reflect(viewDir,worldNormal);
#ifdef INVERTCUBICMAP
coords.y=1.0-coords.y;
#endif
return vec3(reflectionMatrix*vec4(coords,0));
#endif
#ifdef REFLECTIONMAP_PROJECTION
return vec3(reflectionMatrix*(view*worldPos));
#endif
#ifdef REFLECTIONMAP_SKYBOX
return vPositionUVW;
#endif
#ifdef REFLECTIONMAP_EXPLICIT
return vec3(0,0,0);
#endif
}
#endif
#ifdef CAMERACOLORGRADING
uniform sampler2D cameraColorGrading2DSampler;
uniform vec4 vCameraColorGradingInfos;
uniform vec4 vCameraColorGradingScaleOffset;
#endif
#ifdef CAMERACOLORCURVES
uniform vec4 vCameraColorCurveNeutral;
uniform vec4 vCameraColorCurvePositive;
uniform vec4 vCameraColorCurveNegative;
#endif


#ifdef SHADOWS
#ifndef SHADOWFULLFLOAT
float unpack(vec4 color)
{
const vec4 bit_shift=vec4(1.0/(255.0*255.0*255.0),1.0/(255.0*255.0),1.0/255.0,1.0);
return dot(color,bit_shift);
}
#endif
uniform vec2 depthValues;
float computeShadowCube(vec3 lightPosition,samplerCube shadowSampler,float darkness,float bias)
{
vec3 directionToLight=vPositionW-lightPosition;
float depth=length(directionToLight);
depth=clamp(depth,0.,1.0);
directionToLight=normalize(directionToLight);
directionToLight.y =-directionToLight.y;
#ifndef SHADOWFULLFLOAT
float shadow=unpack(textureCube(shadowSampler,directionToLight))+bias;
#else
float shadow=textureCube(shadowSampler,directionToLight).x+bias;
#endif
if (depth>shadow)
{
#ifdef OVERLOADEDSHADOWVALUES
return mix(1.0,darkness,vOverloadedShadowIntensity.x);
#else
return darkness;
#endif
}
return 1.0;
}
float computeShadowWithPCFCube(vec3 lightPosition,samplerCube shadowSampler,float mapSize,float bias,float darkness)
{
vec3 directionToLight=vPositionW-lightPosition;
float depth=length(directionToLight);
depth=(depth-depthValues.x)/(depthValues.y-depthValues.x);
depth=clamp(depth,0.,1.0);
directionToLight=normalize(directionToLight);
directionToLight.y=-directionToLight.y;
float visibility=1.;
vec3 poissonDisk[4];
poissonDisk[0]=vec3(-1.0,1.0,-1.0);
poissonDisk[1]=vec3(1.0,-1.0,-1.0);
poissonDisk[2]=vec3(-1.0,-1.0,-1.0);
poissonDisk[3]=vec3(1.0,-1.0,1.0);

float biasedDepth=depth-bias;
#ifndef SHADOWFULLFLOAT
if (unpack(textureCube(shadowSampler,directionToLight+poissonDisk[0]*mapSize))<biasedDepth) visibility-=0.25;
if (unpack(textureCube(shadowSampler,directionToLight+poissonDisk[1]*mapSize))<biasedDepth) visibility-=0.25;
if (unpack(textureCube(shadowSampler,directionToLight+poissonDisk[2]*mapSize))<biasedDepth) visibility-=0.25;
if (unpack(textureCube(shadowSampler,directionToLight+poissonDisk[3]*mapSize))<biasedDepth) visibility-=0.25;
#else
if (textureCube(shadowSampler,directionToLight+poissonDisk[0]*mapSize).x<biasedDepth) visibility-=0.25;
if (textureCube(shadowSampler,directionToLight+poissonDisk[1]*mapSize).x<biasedDepth) visibility-=0.25;
if (textureCube(shadowSampler,directionToLight+poissonDisk[2]*mapSize).x<biasedDepth) visibility-=0.25;
if (textureCube(shadowSampler,directionToLight+poissonDisk[3]*mapSize).x<biasedDepth) visibility-=0.25;
#endif
#ifdef OVERLOADEDSHADOWVALUES
return min(1.0,mix(1.0,visibility+darkness,vOverloadedShadowIntensity.x));
#else
return min(1.0,visibility+darkness);
#endif
}
float computeShadow(vec4 vPositionFromLight,sampler2D shadowSampler,float darkness,float bias)
{
vec3 depth=vPositionFromLight.xyz/vPositionFromLight.w;
depth=0.5*depth+vec3(0.5);
vec2 uv=depth.xy;
if (uv.x<0. || uv.x>1.0 || uv.y<0. || uv.y>1.0)
{
return 1.0;
}
#ifndef SHADOWFULLFLOAT
float shadow=unpack(texture2D(shadowSampler,uv))+bias;
#else
float shadow=texture2D(shadowSampler,uv).x+bias;
#endif
if (depth.z>shadow)
{
#ifdef OVERLOADEDSHADOWVALUES
return mix(1.0,darkness,vOverloadedShadowIntensity.x);
#else
return darkness;
#endif
}
return 1.;
}
float computeShadowWithPCF(vec4 vPositionFromLight,sampler2D shadowSampler,float mapSize,float bias,float darkness)
{
vec3 depth=vPositionFromLight.xyz/vPositionFromLight.w;
depth=0.5*depth+vec3(0.5);
vec2 uv=depth.xy;
if (uv.x<0. || uv.x>1.0 || uv.y<0. || uv.y>1.0)
{
return 1.0;
}
float visibility=1.;
vec2 poissonDisk[4];
poissonDisk[0]=vec2(-0.94201624,-0.39906216);
poissonDisk[1]=vec2(0.94558609,-0.76890725);
poissonDisk[2]=vec2(-0.094184101,-0.92938870);
poissonDisk[3]=vec2(0.34495938,0.29387760);

float biasedDepth=depth.z-bias;
#ifndef SHADOWFULLFLOAT
if (unpack(texture2D(shadowSampler,uv+poissonDisk[0]*mapSize))<biasedDepth) visibility-=0.25;
if (unpack(texture2D(shadowSampler,uv+poissonDisk[1]*mapSize))<biasedDepth) visibility-=0.25;
if (unpack(texture2D(shadowSampler,uv+poissonDisk[2]*mapSize))<biasedDepth) visibility-=0.25;
if (unpack(texture2D(shadowSampler,uv+poissonDisk[3]*mapSize))<biasedDepth) visibility-=0.25;
#else
if (texture2D(shadowSampler,uv+poissonDisk[0]*mapSize).x<biasedDepth) visibility-=0.25;
if (texture2D(shadowSampler,uv+poissonDisk[1]*mapSize).x<biasedDepth) visibility-=0.25;
if (texture2D(shadowSampler,uv+poissonDisk[2]*mapSize).x<biasedDepth) visibility-=0.25;
if (texture2D(shadowSampler,uv+poissonDisk[3]*mapSize).x<biasedDepth) visibility-=0.25;
#endif
#ifdef OVERLOADEDSHADOWVALUES
return min(1.0,mix(1.0,visibility+darkness,vOverloadedShadowIntensity.x));
#else
return min(1.0,visibility+darkness);
#endif
}
#ifndef SHADOWFULLFLOAT

float unpackHalf(vec2 color)
{
return color.x+(color.y/255.0);
}
#endif
float linstep(float low,float high,float v) {
return clamp((v-low)/(high-low),0.0,1.0);
}
float ChebychevInequality(vec2 moments,float compare,float bias)
{
float p=smoothstep(compare-bias,compare,moments.x);
float variance=max(moments.y-moments.x*moments.x,0.02);
float d=compare-moments.x;
float p_max=linstep(0.2,1.0,variance/(variance+d*d));
return clamp(max(p,p_max),0.0,1.0);
}
float computeShadowWithVSM(vec4 vPositionFromLight,sampler2D shadowSampler,float bias,float darkness)
{
vec3 depth=vPositionFromLight.xyz/vPositionFromLight.w;
depth=0.5*depth+vec3(0.5);
vec2 uv=depth.xy;
if (uv.x<0. || uv.x>1.0 || uv.y<0. || uv.y>1.0 || depth.z>=1.0)
{
return 1.0;
}
vec4 texel=texture2D(shadowSampler,uv);
#ifndef SHADOWFULLFLOAT
vec2 moments=vec2(unpackHalf(texel.xy),unpackHalf(texel.zw));
#else
vec2 moments=texel.xy;
#endif
#ifdef OVERLOADEDSHADOWVALUES
return min(1.0,mix(1.0,1.0-ChebychevInequality(moments,depth.z,bias)+darkness,vOverloadedShadowIntensity.x));
#else
return min(1.0,1.0-ChebychevInequality(moments,depth.z,bias)+darkness);
#endif
}
#endif

#define RECIPROCAL_PI2 0.15915494
#define FRESNEL_MAXIMUM_ON_ROUGH 0.25

const float kPi=3.1415926535897932384626433832795;
const float kRougnhessToAlphaScale=0.1;
const float kRougnhessToAlphaOffset=0.29248125;
float Square(float value)
{
return value*value;
}
float getLuminance(vec3 color)
{
return clamp(dot(color,vec3(0.2126,0.7152,0.0722)),0.,1.);
}
float convertRoughnessToAverageSlope(float roughness)
{

const float kMinimumVariance=0.0005;
float alphaG=Square(roughness)+kMinimumVariance;
return alphaG;
}

float getMipMapIndexFromAverageSlope(float maxMipLevel,float alpha)
{







float mip=kRougnhessToAlphaOffset+maxMipLevel+(maxMipLevel*kRougnhessToAlphaScale*log2(alpha));
return clamp(mip,0.,maxMipLevel);
}
float getMipMapIndexFromAverageSlopeWithPMREM(float maxMipLevel,float alphaG)
{
float specularPower=clamp(2./alphaG-2.,0.000001,2048.);

return clamp(- 0.5*log2(specularPower)+5.5,0.,maxMipLevel);
}

float smithVisibilityG1_TrowbridgeReitzGGX(float dot,float alphaG)
{
float tanSquared=(1.0-dot*dot)/(dot*dot);
return 2.0/(1.0+sqrt(1.0+alphaG*alphaG*tanSquared));
}
float smithVisibilityG_TrowbridgeReitzGGX_Walter(float NdotL,float NdotV,float alphaG)
{
return smithVisibilityG1_TrowbridgeReitzGGX(NdotL,alphaG)*smithVisibilityG1_TrowbridgeReitzGGX(NdotV,alphaG);
}


float normalDistributionFunction_TrowbridgeReitzGGX(float NdotH,float alphaG)
{



float a2=Square(alphaG);
float d=NdotH*NdotH*(a2-1.0)+1.0;
return a2/(kPi*d*d);
}
vec3 fresnelSchlickGGX(float VdotH,vec3 reflectance0,vec3 reflectance90)
{
return reflectance0+(reflectance90-reflectance0)*pow(clamp(1.0-VdotH,0.,1.),5.0);
}
vec3 FresnelSchlickEnvironmentGGX(float VdotN,vec3 reflectance0,vec3 reflectance90,float smoothness)
{

float weight=mix(FRESNEL_MAXIMUM_ON_ROUGH,1.0,smoothness);
return reflectance0+weight*(reflectance90-reflectance0)*pow(clamp(1.0-VdotN,0.,1.),5.0);
}

vec3 computeSpecularTerm(float NdotH,float NdotL,float NdotV,float VdotH,float roughness,vec3 specularColor,vec3 reflectance90)
{
float alphaG=convertRoughnessToAverageSlope(roughness);
float distribution=normalDistributionFunction_TrowbridgeReitzGGX(NdotH,alphaG);
float visibility=smithVisibilityG_TrowbridgeReitzGGX_Walter(NdotL,NdotV,alphaG);
visibility/=(4.0*NdotL*NdotV); 
vec3 fresnel=fresnelSchlickGGX(VdotH,specularColor,reflectance90);
float specTerm=max(0.,visibility*distribution)*NdotL;
return fresnel*specTerm*kPi; 
}
float computeDiffuseTerm(float NdotL,float NdotV,float VdotH,float roughness)
{


float diffuseFresnelNV=pow(clamp(1.0-NdotL,0.000001,1.),5.0);
float diffuseFresnelNL=pow(clamp(1.0-NdotV,0.000001,1.),5.0);
float diffuseFresnel90=0.5+2.0*VdotH*VdotH*roughness;
float diffuseFresnelTerm =
(1.0+(diffuseFresnel90-1.0)*diffuseFresnelNL) *
(1.0+(diffuseFresnel90-1.0)*diffuseFresnelNV);
return diffuseFresnelTerm*NdotL;


}
float adjustRoughnessFromLightProperties(float roughness,float lightRadius,float lightDistance)
{
#ifdef USEPHYSICALLIGHTFALLOFF

float lightRoughness=lightRadius/lightDistance;

float totalRoughness=clamp(lightRoughness+roughness,0.,1.);
return totalRoughness;
#else
return roughness;
#endif
}
float computeDefaultMicroSurface(float microSurface,vec3 reflectivityColor)
{
float kReflectivityNoAlphaWorkflow_SmoothnessMax=0.95;
float reflectivityLuminance=getLuminance(reflectivityColor);
float reflectivityLuma=sqrt(reflectivityLuminance);
microSurface=reflectivityLuma*kReflectivityNoAlphaWorkflow_SmoothnessMax;
return microSurface;
}
vec3 toLinearSpace(vec3 color)
{
return vec3(pow(color.r,2.2),pow(color.g,2.2),pow(color.b,2.2));
}
vec3 toGammaSpace(vec3 color)
{
return vec3(pow(color.r,1.0/2.2),pow(color.g,1.0/2.2),pow(color.b,1.0/2.2));
}
#ifdef CAMERATONEMAP
vec3 toneMaps(vec3 color)
{
color=max(color,0.0);

color.rgb=color.rgb*vCameraInfos.x;
float tuning=1.5; 


vec3 tonemapped=1.0-exp2(-color.rgb*tuning); 
color.rgb=mix(color.rgb,tonemapped,1.0);
return color;
}
#endif
#ifdef CAMERACONTRAST
vec4 contrasts(vec4 color)
{
color=clamp(color,0.0,1.0);
vec3 resultHighContrast=color.rgb*color.rgb*(3.0-2.0*color.rgb);
float contrast=vCameraInfos.y;
if (contrast<1.0)
{

color.rgb=mix(vec3(0.5,0.5,0.5),color.rgb,contrast);
}
else
{

color.rgb=mix(color.rgb,resultHighContrast,contrast-1.0);
}
return color;
}
#endif
#ifdef CAMERACOLORGRADING
vec4 colorGrades(vec4 color) 
{ 

float sliceContinuous=color.z*vCameraColorGradingInfos.z;
float sliceInteger=floor(sliceContinuous);


float sliceFraction=sliceContinuous-sliceInteger; 

vec2 sliceUV=color.xy*vCameraColorGradingScaleOffset.xy+vCameraColorGradingScaleOffset.zw;


sliceUV.x+=sliceInteger*vCameraColorGradingInfos.w;
vec4 slice0Color=texture2D(cameraColorGrading2DSampler,sliceUV);
sliceUV.x+=vCameraColorGradingInfos.w;
vec4 slice1Color=texture2D(cameraColorGrading2DSampler,sliceUV);
vec3 result=mix(slice0Color.rgb,slice1Color.rgb,sliceFraction);
color.rgb=mix(color.rgb,result,vCameraColorGradingInfos.x);
return color;
}
#endif
#ifdef CAMERACOLORCURVES
const vec3 HDTVRec709_RGBLuminanceCoefficients=vec3(0.2126,0.7152,0.0722);
vec3 applyColorCurves(vec3 original) {
vec3 result=original;



float luma=dot(result.rgb,HDTVRec709_RGBLuminanceCoefficients);
vec2 curveMix=clamp(vec2(luma*3.0-1.5,luma*-3.0+1.5),vec2(0.0,0.0),vec2(1.0,1.0));
vec4 colorCurve=vCameraColorCurveNeutral+curveMix.x*vCameraColorCurvePositive-curveMix.y*vCameraColorCurveNegative;
result.rgb*=colorCurve.rgb;
result.rgb=mix(vec3(luma,luma,luma),result.rgb,colorCurve.a);
return result;
}
#endif
#ifdef USESPHERICALFROMREFLECTIONMAP
uniform vec3 vSphericalX;
uniform vec3 vSphericalY;
uniform vec3 vSphericalZ;
uniform vec3 vSphericalXX;
uniform vec3 vSphericalYY;
uniform vec3 vSphericalZZ;
uniform vec3 vSphericalXY;
uniform vec3 vSphericalYZ;
uniform vec3 vSphericalZX;
vec3 EnvironmentIrradiance(vec3 normal)
{



vec3 result =
vSphericalX*normal.x +
vSphericalY*normal.y +
vSphericalZ*normal.z +
vSphericalXX*normal.x*normal.x +
vSphericalYY*normal.y*normal.y +
vSphericalZZ*normal.z*normal.z +
vSphericalYZ*normal.y*normal.z +
vSphericalZX*normal.z*normal.x +
vSphericalXY*normal.x*normal.y;
return result.rgb;
}
#endif

struct lightingInfo
{
vec3 diffuse;
#ifdef SPECULARTERM
vec3 specular;
#endif
};
float computeDistanceLightFalloff(vec3 lightOffset,float lightDistanceSquared,float range)
{ 
#ifdef USEPHYSICALLIGHTFALLOFF
float lightDistanceFalloff=1.0/((lightDistanceSquared+0.0001));
#else
float lightDistanceFalloff=max(0.,1.0-length(lightOffset)/range);
#endif
return lightDistanceFalloff;
}
float computeDirectionalLightFalloff(vec3 lightDirection,vec3 directionToLightCenterW,float lightAngle,float exponent)
{
float falloff=0.0;
#ifdef USEPHYSICALLIGHTFALLOFF
float cosHalfAngle=cos(lightAngle*0.5);
const float kMinusLog2ConeAngleIntensityRatio=6.64385618977; 





float concentrationKappa=kMinusLog2ConeAngleIntensityRatio/(1.0-cosHalfAngle);


vec4 lightDirectionSpreadSG=vec4(-lightDirection*concentrationKappa,-concentrationKappa);
falloff=exp2(dot(vec4(directionToLightCenterW,1.0),lightDirectionSpreadSG));
#else
float cosAngle=max(0.000000000000001,dot(-lightDirection,directionToLightCenterW));
if (cosAngle>=lightAngle)
{
falloff=max(0.,pow(cosAngle,exponent));
}
#endif
return falloff;
}
lightingInfo computeLighting(vec3 viewDirectionW,vec3 vNormal,vec4 lightData,vec3 diffuseColor,vec3 specularColor,float rangeRadius,float roughness,float NdotV,vec3 reflectance90,out float NdotL) {
lightingInfo result;
vec3 lightDirection;
float attenuation=1.0;
float lightDistance;

if (lightData.w == 0.)
{
vec3 lightOffset=lightData.xyz-vPositionW;
float lightDistanceSquared=dot(lightOffset,lightOffset);
attenuation=computeDistanceLightFalloff(lightOffset,lightDistanceSquared,rangeRadius);
lightDistance=sqrt(lightDistanceSquared);
lightDirection=normalize(lightOffset);
}

else
{
lightDistance=length(-lightData.xyz);
lightDirection=normalize(-lightData.xyz);
}

roughness=adjustRoughnessFromLightProperties(roughness,rangeRadius,lightDistance);

vec3 H=normalize(viewDirectionW+lightDirection);
NdotL=max(0.00000000001,dot(vNormal,lightDirection));
float VdotH=clamp(0.00000000001,1.0,dot(viewDirectionW,H));
float diffuseTerm=computeDiffuseTerm(NdotL,NdotV,VdotH,roughness);
result.diffuse=diffuseTerm*diffuseColor*attenuation;
#ifdef SPECULARTERM

float NdotH=max(0.00000000001,dot(vNormal,H));
vec3 specTerm=computeSpecularTerm(NdotH,NdotL,NdotV,VdotH,roughness,specularColor,reflectance90);
result.specular=specTerm*attenuation;
#endif
return result;
}
lightingInfo computeSpotLighting(vec3 viewDirectionW,vec3 vNormal,vec4 lightData,vec4 lightDirection,vec3 diffuseColor,vec3 specularColor,float rangeRadius,float roughness,float NdotV,vec3 reflectance90,out float NdotL) {
lightingInfo result;
vec3 lightOffset=lightData.xyz-vPositionW;
vec3 directionToLightCenterW=normalize(lightOffset);

float lightDistanceSquared=dot(lightOffset,lightOffset);
float attenuation=computeDistanceLightFalloff(lightOffset,lightDistanceSquared,rangeRadius);

float directionalAttenuation=computeDirectionalLightFalloff(lightDirection.xyz,directionToLightCenterW,lightDirection.w,lightData.w);
attenuation*=directionalAttenuation;

float lightDistance=sqrt(lightDistanceSquared);
roughness=adjustRoughnessFromLightProperties(roughness,rangeRadius,lightDistance);

vec3 H=normalize(viewDirectionW-lightDirection.xyz);
NdotL=max(0.00000000001,dot(vNormal,-lightDirection.xyz));
float VdotH=clamp(dot(viewDirectionW,H),0.00000000001,1.0);
float diffuseTerm=computeDiffuseTerm(NdotL,NdotV,VdotH,roughness);
result.diffuse=diffuseTerm*diffuseColor*attenuation;
#ifdef SPECULARTERM

float NdotH=max(0.00000000001,dot(vNormal,H));
vec3 specTerm=computeSpecularTerm(NdotH,NdotL,NdotV,VdotH,roughness,specularColor,reflectance90);
result.specular=specTerm*attenuation;
#endif
return result;
}
lightingInfo computeHemisphericLighting(vec3 viewDirectionW,vec3 vNormal,vec4 lightData,vec3 diffuseColor,vec3 specularColor,vec3 groundColor,float roughness,float NdotV,vec3 reflectance90,out float NdotL) {
lightingInfo result;



NdotL=dot(vNormal,lightData.xyz)*0.5+0.5;
result.diffuse=mix(groundColor,diffuseColor,NdotL);
#ifdef SPECULARTERM

vec3 lightVectorW=normalize(lightData.xyz);
vec3 H=normalize(viewDirectionW+lightVectorW);
float NdotH=max(0.00000000001,dot(vNormal,H));
NdotL=max(0.00000000001,NdotL);
float VdotH=clamp(0.00000000001,1.0,dot(viewDirectionW,H));
vec3 specTerm=computeSpecularTerm(NdotH,NdotL,NdotV,VdotH,roughness,specularColor,reflectance90);
result.specular=specTerm;
#endif
return result;
}
mat3 transposeMat3(mat3 inMatrix) {
vec3 i0=inMatrix[0];
vec3 i1=inMatrix[1];
vec3 i2=inMatrix[2];
mat3 outMatrix=mat3(
vec3(i0.x,i1.x,i2.x),
vec3(i0.y,i1.y,i2.y),
vec3(i0.z,i1.z,i2.z)
);
return outMatrix;
}
#ifdef BUMP
varying vec2 vBumpUV;
uniform vec3 vBumpInfos;
uniform sampler2D bumpSampler;

mat3 cotangent_frame(vec3 normal,vec3 p,vec2 uv)
{

vec3 dp1=dFdx(p);
vec3 dp2=dFdy(p);
vec2 duv1=dFdx(uv);
vec2 duv2=dFdy(uv);

vec3 dp2perp=cross(dp2,normal);
vec3 dp1perp=cross(normal,dp1);
vec3 tangent=dp2perp*duv1.x+dp1perp*duv2.x;
vec3 binormal=dp2perp*duv1.y+dp1perp*duv2.y;

float invmax=inversesqrt(max(dot(tangent,tangent),dot(binormal,binormal)));
return mat3(tangent*invmax,binormal*invmax,normal);
}
vec3 perturbNormal(vec3 viewDir,mat3 cotangentFrame,vec2 uv)
{
vec3 map=texture2D(bumpSampler,uv).xyz;
#ifdef INVERTNORMALMAPX
map.x=1.0-map.x;
#endif
#ifdef INVERTNORMALMAPY
map.y=1.0-map.y;
#endif
map=map*255./127.-128./127.;
return normalize(cotangentFrame*map);
}
#ifdef PARALLAX
const float minSamples=4.;
const float maxSamples=15.;
const int iMaxSamples=15;

vec2 parallaxOcclusion(vec3 vViewDirCoT,vec3 vNormalCoT,vec2 texCoord,float parallaxScale) {
float parallaxLimit=length(vViewDirCoT.xy)/vViewDirCoT.z;
parallaxLimit*=parallaxScale;
vec2 vOffsetDir=normalize(vViewDirCoT.xy);
vec2 vMaxOffset=vOffsetDir*parallaxLimit;
float numSamples=maxSamples+(dot(vViewDirCoT,vNormalCoT)*(minSamples-maxSamples));
float stepSize=1.0/numSamples;

float currRayHeight=1.0;
vec2 vCurrOffset=vec2(0,0);
vec2 vLastOffset=vec2(0,0);
float lastSampledHeight=1.0;
float currSampledHeight=1.0;
for (int i=0; i<iMaxSamples; i++)
{
currSampledHeight=texture2D(bumpSampler,vBumpUV+vCurrOffset).w;

if (currSampledHeight>currRayHeight)
{
float delta1=currSampledHeight-currRayHeight;
float delta2=(currRayHeight+stepSize)-lastSampledHeight;
float ratio=delta1/(delta1+delta2);
vCurrOffset=(ratio)* vLastOffset+(1.0-ratio)*vCurrOffset;

break;
}
else
{
currRayHeight-=stepSize;
vLastOffset=vCurrOffset;
vCurrOffset+=stepSize*vMaxOffset;
lastSampledHeight=currSampledHeight;
}
}
return vCurrOffset;
}
vec2 parallaxOffset(vec3 viewDir,float heightScale)
{

float height=texture2D(bumpSampler,vBumpUV).w;
vec2 texCoordOffset=heightScale*viewDir.xy*height;
return -texCoordOffset;
}
#endif
#endif
#ifdef CLIPPLANE
varying float fClipDistance;
#endif
#ifdef LOGARITHMICDEPTH
uniform float logarithmicDepthConstant;
varying float vFragmentDepth;
#endif

#ifdef FOG
#define FOGMODE_NONE 0.
#define FOGMODE_EXP 1.
#define FOGMODE_EXP2 2.
#define FOGMODE_LINEAR 3.
#define E 2.71828
uniform vec4 vFogInfos;
uniform vec3 vFogColor;
varying float fFogDistance;
float CalcFogFactor()
{
float fogCoeff=1.0;
float fogStart=vFogInfos.y;
float fogEnd=vFogInfos.z;
float fogDensity=vFogInfos.w;
if (FOGMODE_LINEAR == vFogInfos.x)
{
fogCoeff=(fogEnd-fFogDistance)/(fogEnd-fogStart);
}
else if (FOGMODE_EXP == vFogInfos.x)
{
fogCoeff=1.0/pow(E,fFogDistance*fogDensity);
}
else if (FOGMODE_EXP2 == vFogInfos.x)
{
fogCoeff=1.0/pow(E,fFogDistance*fFogDistance*fogDensity*fogDensity);
}
return clamp(fogCoeff,0.0,1.0);
}
#endif
void main(void) {
#ifdef CLIPPLANE
if (fClipDistance>0.0)
{
discard;
}
#endif
vec3 viewDirectionW=normalize(vEyePosition-vPositionW);

#ifdef NORMAL
vec3 normalW=normalize(vNormalW);
#else
vec3 normalW=vec3(1.0,1.0,1.0);
#endif
vec2 uvOffset=vec2(0.0,0.0);
#if defined(BUMP) || defined(PARALLAX)
mat3 TBN=cotangent_frame(normalW*vBumpInfos.y,-viewDirectionW,vBumpUV);
#endif
#ifdef PARALLAX
mat3 invTBN=transposeMat3(TBN);
#ifdef PARALLAXOCCLUSION
uvOffset=parallaxOcclusion(invTBN*-viewDirectionW,invTBN*normalW,vBumpUV,vBumpInfos.z);
#else
uvOffset=parallaxOffset(invTBN*viewDirectionW,vBumpInfos.z);
#endif
#endif
#ifdef BUMP
normalW=perturbNormal(viewDirectionW,TBN,vBumpUV+uvOffset);
#endif

vec4 surfaceAlbedo=vec4(1.,1.,1.,1.);
vec3 surfaceAlbedoContribution=vAlbedoColor.rgb;

float alpha=vAlbedoColor.a;
#ifdef ALBEDO
surfaceAlbedo=texture2D(albedoSampler,vAlbedoUV+uvOffset);
surfaceAlbedo=vec4(toLinearSpace(surfaceAlbedo.rgb),surfaceAlbedo.a);
#ifndef LINKREFRACTIONTOTRANSPARENCY
#ifdef ALPHATEST
if (surfaceAlbedo.a<0.4)
discard;
#endif
#endif
#ifdef ALPHAFROMALBEDO
alpha*=surfaceAlbedo.a;
#endif
surfaceAlbedo.rgb*=vAlbedoInfos.y;
#else

surfaceAlbedo.rgb=surfaceAlbedoContribution;
surfaceAlbedoContribution=vec3(1.,1.,1.);
#endif
#ifdef VERTEXCOLOR
surfaceAlbedo.rgb*=vColor.rgb;
#endif
#ifdef OVERLOADEDVALUES
surfaceAlbedo.rgb=mix(surfaceAlbedo.rgb,vOverloadedAlbedo,vOverloadedIntensity.y);
#endif

vec3 ambientColor=vec3(1.,1.,1.);
#ifdef AMBIENT
ambientColor=texture2D(ambientSampler,vAmbientUV+uvOffset).rgb*vAmbientInfos.y;
ambientColor=vec3(1.,1.,1.)-((vec3(1.,1.,1.)-ambientColor)*vAmbientInfos.z);
#ifdef OVERLOADEDVALUES
ambientColor.rgb=mix(ambientColor.rgb,vOverloadedAmbient,vOverloadedIntensity.x);
#endif
#endif

float microSurface=vReflectivityColor.a;
vec3 surfaceReflectivityColor=vReflectivityColor.rgb;
#ifdef OVERLOADEDVALUES
surfaceReflectivityColor.rgb=mix(surfaceReflectivityColor.rgb,vOverloadedReflectivity,vOverloadedIntensity.z);
#endif
#ifdef REFLECTIVITY
vec4 surfaceReflectivityColorMap=texture2D(reflectivitySampler,vReflectivityUV+uvOffset);
surfaceReflectivityColor=surfaceReflectivityColorMap.rgb;
surfaceReflectivityColor=toLinearSpace(surfaceReflectivityColor);
#ifdef OVERLOADEDVALUES
surfaceReflectivityColor=mix(surfaceReflectivityColor,vOverloadedReflectivity,vOverloadedIntensity.z);
#endif
#ifdef MICROSURFACEFROMREFLECTIVITYMAP
microSurface=surfaceReflectivityColorMap.a;
#else
#ifdef MICROSURFACEAUTOMATIC
microSurface=computeDefaultMicroSurface(microSurface,surfaceReflectivityColor);
#endif
#endif
#endif
#ifdef METALLICWORKFLOW
vec4 surfaceMetallicColorMap=texture2D(reflectivitySampler,vReflectivityUV+uvOffset);

float metallic=surfaceMetallicColorMap.r; 

vec3 baseColor=surfaceAlbedo.rgb;

surfaceAlbedo.rgb*=(1.0-metallic);


const vec3 DefaultSpecularReflectanceDielectric=vec3(0.04,0.04,0.04);

surfaceReflectivityColor=mix(DefaultSpecularReflectanceDielectric,baseColor,metallic);
#ifdef OVERLOADEDVALUES
surfaceReflectivityColor=mix(surfaceReflectivityColor,vOverloadedReflectivity,vOverloadedIntensity.z);
#endif
#ifdef METALLICROUGHNESSGSTOREINALPHA
microSurface=1.0-surfaceMetallicColorMap.a;
#else
#ifdef METALLICROUGHNESSGSTOREINGREEN
microSurface=1.0-surfaceMetallicColorMap.g;
#endif
#endif
#endif
#ifdef OVERLOADEDVALUES
microSurface=mix(microSurface,vOverloadedMicroSurface.x,vOverloadedMicroSurface.y);
#endif

float NdotV=max(0.00000000001,dot(normalW,viewDirectionW));

microSurface=clamp(microSurface,0.,1.)*0.98;

float roughness=clamp(1.-microSurface,0.000001,1.0);

vec3 lightDiffuseContribution=vec3(0.,0.,0.);
#ifdef OVERLOADEDSHADOWVALUES
vec3 shadowedOnlyLightDiffuseContribution=vec3(1.,1.,1.);
#endif
#ifdef SPECULARTERM
vec3 lightSpecularContribution=vec3(0.,0.,0.);
#endif
float notShadowLevel=1.; 
#ifdef LIGHTMAP
vec3 lightmapColor=texture2D(lightmapSampler,vLightmapUV+uvOffset).rgb*vLightmapInfos.y;
#endif
float NdotL=-1.;
lightingInfo info;

float reflectance=max(max(surfaceReflectivityColor.r,surfaceReflectivityColor.g),surfaceReflectivityColor.b);


float reflectance90=clamp(reflectance*25.0,0.0,1.0);
vec3 specularEnvironmentR0=surfaceReflectivityColor.rgb;
vec3 specularEnvironmentR90=vec3(1.0,1.0,1.0)*reflectance90;
#ifdef LIGHT0
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED0) && defined(LIGHTMAPNOSPECULAR0)

#else
#ifndef SPECULARTERM
vec3 vLightSpecular0=vec3(0.0);
#endif
#ifdef SPOTLIGHT0
info=computeSpotLighting(viewDirectionW,normalW,vLightData0,vLightDirection0,vLightDiffuse0.rgb,vLightSpecular0,vLightDiffuse0.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#ifdef HEMILIGHT0
info=computeHemisphericLighting(viewDirectionW,normalW,vLightData0,vLightDiffuse0.rgb,vLightSpecular0,vLightGround0,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#if defined(POINTLIGHT0) || defined(DIRLIGHT0)
info=computeLighting(viewDirectionW,normalW,vLightData0,vLightDiffuse0.rgb,vLightSpecular0,vLightDiffuse0.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#endif
#ifdef SHADOW0
#ifdef SHADOWVSM0
notShadowLevel=computeShadowWithVSM(vPositionFromLight0,shadowSampler0,shadowsInfo0.z,shadowsInfo0.x);
#else
#ifdef SHADOWPCF0
#if defined(POINTLIGHT0)
notShadowLevel=computeShadowWithPCFCube(vLightData0.xyz,shadowSampler0,shadowsInfo0.y,shadowsInfo0.z,shadowsInfo0.x);
#else
notShadowLevel=computeShadowWithPCF(vPositionFromLight0,shadowSampler0,shadowsInfo0.y,shadowsInfo0.z,shadowsInfo0.x);
#endif
#else
#if defined(POINTLIGHT0)
notShadowLevel=computeShadowCube(vLightData0.xyz,shadowSampler0,shadowsInfo0.x,shadowsInfo0.z);
#else
notShadowLevel=computeShadow(vPositionFromLight0,shadowSampler0,shadowsInfo0.x,shadowsInfo0.z);
#endif
#endif
#endif
#else
notShadowLevel=1.;
#endif
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED0)
lightDiffuseContribution+=lightmapColor*notShadowLevel;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR0
lightSpecularContribution+=info.specular*notShadowLevel*lightmapColor;
#endif
#endif
#else
lightDiffuseContribution+=info.diffuse*notShadowLevel;
#ifdef OVERLOADEDSHADOWVALUES
if (NdotL<0.000000000011)
{
notShadowLevel=1.;
}
shadowedOnlyLightDiffuseContribution*=notShadowLevel;
#endif
#ifdef SPECULARTERM
lightSpecularContribution+=info.specular*notShadowLevel;
#endif
#endif
#endif
#ifdef LIGHT1
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED1) && defined(LIGHTMAPNOSPECULAR1)

#else
#ifndef SPECULARTERM
vec3 vLightSpecular1=vec3(0.0);
#endif
#ifdef SPOTLIGHT1
info=computeSpotLighting(viewDirectionW,normalW,vLightData1,vLightDirection1,vLightDiffuse1.rgb,vLightSpecular1,vLightDiffuse1.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#ifdef HEMILIGHT1
info=computeHemisphericLighting(viewDirectionW,normalW,vLightData1,vLightDiffuse1.rgb,vLightSpecular1,vLightGround1,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#if defined(POINTLIGHT1) || defined(DIRLIGHT1)
info=computeLighting(viewDirectionW,normalW,vLightData1,vLightDiffuse1.rgb,vLightSpecular1,vLightDiffuse1.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#endif
#ifdef SHADOW1
#ifdef SHADOWVSM1
notShadowLevel=computeShadowWithVSM(vPositionFromLight1,shadowSampler1,shadowsInfo1.z,shadowsInfo1.x);
#else
#ifdef SHADOWPCF1
#if defined(POINTLIGHT1)
notShadowLevel=computeShadowWithPCFCube(vLightData1.xyz,shadowSampler1,shadowsInfo1.y,shadowsInfo1.z,shadowsInfo1.x);
#else
notShadowLevel=computeShadowWithPCF(vPositionFromLight1,shadowSampler1,shadowsInfo1.y,shadowsInfo1.z,shadowsInfo1.x);
#endif
#else
#if defined(POINTLIGHT1)
notShadowLevel=computeShadowCube(vLightData1.xyz,shadowSampler1,shadowsInfo1.x,shadowsInfo1.z);
#else
notShadowLevel=computeShadow(vPositionFromLight1,shadowSampler1,shadowsInfo1.x,shadowsInfo1.z);
#endif
#endif
#endif
#else
notShadowLevel=1.;
#endif
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED1)
lightDiffuseContribution+=lightmapColor*notShadowLevel;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR1
lightSpecularContribution+=info.specular*notShadowLevel*lightmapColor;
#endif
#endif
#else
lightDiffuseContribution+=info.diffuse*notShadowLevel;
#ifdef OVERLOADEDSHADOWVALUES
if (NdotL<0.000000000011)
{
notShadowLevel=1.;
}
shadowedOnlyLightDiffuseContribution*=notShadowLevel;
#endif
#ifdef SPECULARTERM
lightSpecularContribution+=info.specular*notShadowLevel;
#endif
#endif
#endif
#ifdef LIGHT2
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED2) && defined(LIGHTMAPNOSPECULAR2)

#else
#ifndef SPECULARTERM
vec3 vLightSpecular2=vec3(0.0);
#endif
#ifdef SPOTLIGHT2
info=computeSpotLighting(viewDirectionW,normalW,vLightData2,vLightDirection2,vLightDiffuse2.rgb,vLightSpecular2,vLightDiffuse2.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#ifdef HEMILIGHT2
info=computeHemisphericLighting(viewDirectionW,normalW,vLightData2,vLightDiffuse2.rgb,vLightSpecular2,vLightGround2,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#if defined(POINTLIGHT2) || defined(DIRLIGHT2)
info=computeLighting(viewDirectionW,normalW,vLightData2,vLightDiffuse2.rgb,vLightSpecular2,vLightDiffuse2.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#endif
#ifdef SHADOW2
#ifdef SHADOWVSM2
notShadowLevel=computeShadowWithVSM(vPositionFromLight2,shadowSampler2,shadowsInfo2.z,shadowsInfo2.x);
#else
#ifdef SHADOWPCF2
#if defined(POINTLIGHT2)
notShadowLevel=computeShadowWithPCFCube(vLightData2.xyz,shadowSampler2,shadowsInfo2.y,shadowsInfo2.z,shadowsInfo2.x);
#else
notShadowLevel=computeShadowWithPCF(vPositionFromLight2,shadowSampler2,shadowsInfo2.y,shadowsInfo2.z,shadowsInfo2.x);
#endif
#else
#if defined(POINTLIGHT2)
notShadowLevel=computeShadowCube(vLightData2.xyz,shadowSampler2,shadowsInfo2.x,shadowsInfo2.z);
#else
notShadowLevel=computeShadow(vPositionFromLight2,shadowSampler2,shadowsInfo2.x,shadowsInfo2.z);
#endif
#endif
#endif
#else
notShadowLevel=1.;
#endif
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED2)
lightDiffuseContribution+=lightmapColor*notShadowLevel;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR2
lightSpecularContribution+=info.specular*notShadowLevel*lightmapColor;
#endif
#endif
#else
lightDiffuseContribution+=info.diffuse*notShadowLevel;
#ifdef OVERLOADEDSHADOWVALUES
if (NdotL<0.000000000011)
{
notShadowLevel=1.;
}
shadowedOnlyLightDiffuseContribution*=notShadowLevel;
#endif
#ifdef SPECULARTERM
lightSpecularContribution+=info.specular*notShadowLevel;
#endif
#endif
#endif
#ifdef LIGHT3
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED3) && defined(LIGHTMAPNOSPECULAR3)

#else
#ifndef SPECULARTERM
vec3 vLightSpecular3=vec3(0.0);
#endif
#ifdef SPOTLIGHT3
info=computeSpotLighting(viewDirectionW,normalW,vLightData3,vLightDirection3,vLightDiffuse3.rgb,vLightSpecular3,vLightDiffuse3.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#ifdef HEMILIGHT3
info=computeHemisphericLighting(viewDirectionW,normalW,vLightData3,vLightDiffuse3.rgb,vLightSpecular3,vLightGround3,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#if defined(POINTLIGHT3) || defined(DIRLIGHT3)
info=computeLighting(viewDirectionW,normalW,vLightData3,vLightDiffuse3.rgb,vLightSpecular3,vLightDiffuse3.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#endif
#ifdef SHADOW3
#ifdef SHADOWVSM3
notShadowLevel=computeShadowWithVSM(vPositionFromLight3,shadowSampler3,shadowsInfo3.z,shadowsInfo3.x);
#else
#ifdef SHADOWPCF3
#if defined(POINTLIGHT3)
notShadowLevel=computeShadowWithPCFCube(vLightData3.xyz,shadowSampler3,shadowsInfo3.y,shadowsInfo3.z,shadowsInfo3.x);
#else
notShadowLevel=computeShadowWithPCF(vPositionFromLight3,shadowSampler3,shadowsInfo3.y,shadowsInfo3.z,shadowsInfo3.x);
#endif
#else
#if defined(POINTLIGHT3)
notShadowLevel=computeShadowCube(vLightData3.xyz,shadowSampler3,shadowsInfo3.x,shadowsInfo3.z);
#else
notShadowLevel=computeShadow(vPositionFromLight3,shadowSampler3,shadowsInfo3.x,shadowsInfo3.z);
#endif
#endif
#endif
#else
notShadowLevel=1.;
#endif
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED3)
lightDiffuseContribution+=lightmapColor*notShadowLevel;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR3
lightSpecularContribution+=info.specular*notShadowLevel*lightmapColor;
#endif
#endif
#else
lightDiffuseContribution+=info.diffuse*notShadowLevel;
#ifdef OVERLOADEDSHADOWVALUES
if (NdotL<0.000000000011)
{
notShadowLevel=1.;
}
shadowedOnlyLightDiffuseContribution*=notShadowLevel;
#endif
#ifdef SPECULARTERM
lightSpecularContribution+=info.specular*notShadowLevel;
#endif
#endif
#endif
#ifdef LIGHT4
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED4) && defined(LIGHTMAPNOSPECULAR4)

#else
#ifndef SPECULARTERM
vec3 vLightSpecular4=vec3(0.0);
#endif
#ifdef SPOTLIGHT4
info=computeSpotLighting(viewDirectionW,normalW,vLightData4,vLightDirection4,vLightDiffuse4.rgb,vLightSpecular4,vLightDiffuse4.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#ifdef HEMILIGHT4
info=computeHemisphericLighting(viewDirectionW,normalW,vLightData4,vLightDiffuse4.rgb,vLightSpecular4,vLightGround4,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#if defined(POINTLIGHT4) || defined(DIRLIGHT4)
info=computeLighting(viewDirectionW,normalW,vLightData4,vLightDiffuse4.rgb,vLightSpecular4,vLightDiffuse4.a,roughness,NdotV,specularEnvironmentR90,NdotL);
#endif
#endif
#ifdef SHADOW4
#ifdef SHADOWVSM4
notShadowLevel=computeShadowWithVSM(vPositionFromLight4,shadowSampler4,shadowsInfo4.z,shadowsInfo4.x);
#else
#ifdef SHADOWPCF4
#if defined(POINTLIGHT4)
notShadowLevel=computeShadowWithPCFCube(vLightData4.xyz,shadowSampler4,shadowsInfo4.y,shadowsInfo4.z,shadowsInfo4.x);
#else
notShadowLevel=computeShadowWithPCF(vPositionFromLight4,shadowSampler4,shadowsInfo4.y,shadowsInfo4.z,shadowsInfo4.x);
#endif
#else
#if defined(POINTLIGHT4)
notShadowLevel=computeShadowCube(vLightData4.xyz,shadowSampler4,shadowsInfo4.x,shadowsInfo4.z);
#else
notShadowLevel=computeShadow(vPositionFromLight4,shadowSampler4,shadowsInfo4.x,shadowsInfo4.z);
#endif
#endif
#endif
#else
notShadowLevel=1.;
#endif
#if defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED4)
lightDiffuseContribution+=lightmapColor*notShadowLevel;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR4
lightSpecularContribution+=info.specular*notShadowLevel*lightmapColor;
#endif
#endif
#else
lightDiffuseContribution+=info.diffuse*notShadowLevel;
#ifdef OVERLOADEDSHADOWVALUES
if (NdotL<0.000000000011)
{
notShadowLevel=1.;
}
shadowedOnlyLightDiffuseContribution*=notShadowLevel;
#endif
#ifdef SPECULARTERM
lightSpecularContribution+=info.specular*notShadowLevel;
#endif
#endif
#endif

#ifdef SPECULARTERM
lightSpecularContribution*=vLightingIntensity.w;
#endif
#ifdef OPACITY
vec4 opacityMap=texture2D(opacitySampler,vOpacityUV+uvOffset);
#ifdef OPACITYRGB
opacityMap.rgb=opacityMap.rgb*vec3(0.3,0.59,0.11);
alpha*=(opacityMap.x+opacityMap.y+opacityMap.z)* vOpacityInfos.y;
#else
alpha*=opacityMap.a*vOpacityInfos.y;
#endif
#endif
#ifdef VERTEXALPHA
alpha*=vColor.a;
#endif
#ifdef OPACITYFRESNEL
float opacityFresnelTerm=computeFresnelTerm(viewDirectionW,normalW,opacityParts.z,opacityParts.w);
alpha+=opacityParts.x*(1.0-opacityFresnelTerm)+opacityFresnelTerm*opacityParts.y;
#endif

vec3 surfaceRefractionColor=vec3(0.,0.,0.);

#ifdef LODBASEDMICROSFURACE
float alphaG=convertRoughnessToAverageSlope(roughness);
#endif
#ifdef REFRACTION
vec3 refractionVector=refract(-viewDirectionW,normalW,vRefractionInfos.y);
#ifdef LODBASEDMICROSFURACE
#ifdef USEPMREMREFRACTION
float lodRefraction=getMipMapIndexFromAverageSlopeWithPMREM(vMicrosurfaceTextureLods.y,alphaG);
#else
float lodRefraction=getMipMapIndexFromAverageSlope(vMicrosurfaceTextureLods.y,alphaG);
#endif
#else
float biasRefraction=(vMicrosurfaceTextureLods.y+2.)*(1.0-microSurface);
#endif
#ifdef REFRACTIONMAP_3D
refractionVector.y=refractionVector.y*vRefractionInfos.w;
if (dot(refractionVector,viewDirectionW)<1.0)
{
#ifdef LODBASEDMICROSFURACE
#ifdef USEPMREMREFRACTION

if ((vMicrosurfaceTextureLods.y-lodRefraction)>4.0)
{

float scaleRefraction=1.-exp2(lodRefraction)/exp2(vMicrosurfaceTextureLods.y); 
float maxRefraction=max(max(abs(refractionVector.x),abs(refractionVector.y)),abs(refractionVector.z));
if (abs(refractionVector.x) != maxRefraction) refractionVector.x*=scaleRefraction;
if (abs(refractionVector.y) != maxRefraction) refractionVector.y*=scaleRefraction;
if (abs(refractionVector.z) != maxRefraction) refractionVector.z*=scaleRefraction;
}
#endif
surfaceRefractionColor=textureCubeLodEXT(refractionCubeSampler,refractionVector,lodRefraction).rgb*vRefractionInfos.x;
#else
surfaceRefractionColor=textureCube(refractionCubeSampler,refractionVector,biasRefraction).rgb*vRefractionInfos.x;
#endif
}
#ifndef REFRACTIONMAPINLINEARSPACE
surfaceRefractionColor=toLinearSpace(surfaceRefractionColor.rgb);
#endif
#else
vec3 vRefractionUVW=vec3(refractionMatrix*(view*vec4(vPositionW+refractionVector*vRefractionInfos.z,1.0)));
vec2 refractionCoords=vRefractionUVW.xy/vRefractionUVW.z;
refractionCoords.y=1.0-refractionCoords.y;
#ifdef LODBASEDMICROSFURACE
surfaceRefractionColor=texture2DLodEXT(refraction2DSampler,refractionCoords,lodRefraction).rgb*vRefractionInfos.x;
#else
surfaceRefractionColor=texture2D(refraction2DSampler,refractionCoords,biasRefraction).rgb*vRefractionInfos.x;
#endif 
surfaceRefractionColor=toLinearSpace(surfaceRefractionColor.rgb);
#endif
#endif

vec3 environmentRadiance=vReflectionColor.rgb;
vec3 environmentIrradiance=vReflectionColor.rgb;
#ifdef REFLECTION
vec3 vReflectionUVW=computeReflectionCoords(vec4(vPositionW,1.0),normalW);
#ifdef LODBASEDMICROSFURACE
#ifdef USEPMREMREFLECTION
float lodReflection=getMipMapIndexFromAverageSlopeWithPMREM(vMicrosurfaceTextureLods.x,alphaG);
#else
float lodReflection=getMipMapIndexFromAverageSlope(vMicrosurfaceTextureLods.x,alphaG);
#endif
#else
float biasReflection=(vMicrosurfaceTextureLods.x+2.)*(1.0-microSurface);
#endif
#ifdef REFLECTIONMAP_3D
#ifdef LODBASEDMICROSFURACE
#ifdef USEPMREMREFLECTION

if ((vMicrosurfaceTextureLods.y-lodReflection)>4.0)
{

float scaleReflection=1.-exp2(lodReflection)/exp2(vMicrosurfaceTextureLods.x); 
float maxReflection=max(max(abs(vReflectionUVW.x),abs(vReflectionUVW.y)),abs(vReflectionUVW.z));
if (abs(vReflectionUVW.x) != maxReflection) vReflectionUVW.x*=scaleReflection;
if (abs(vReflectionUVW.y) != maxReflection) vReflectionUVW.y*=scaleReflection;
if (abs(vReflectionUVW.z) != maxReflection) vReflectionUVW.z*=scaleReflection;
}
#endif
environmentRadiance=textureCubeLodEXT(reflectionCubeSampler,vReflectionUVW,lodReflection).rgb*vReflectionInfos.x;
#else
environmentRadiance=textureCube(reflectionCubeSampler,vReflectionUVW,biasReflection).rgb*vReflectionInfos.x;
#endif
#ifdef USESPHERICALFROMREFLECTIONMAP
#ifndef REFLECTIONMAP_SKYBOX
vec3 normalEnvironmentSpace=(reflectionMatrix*vec4(normalW,1)).xyz;
environmentIrradiance=EnvironmentIrradiance(normalEnvironmentSpace);
#endif
#else
environmentRadiance=toLinearSpace(environmentRadiance.rgb);
environmentIrradiance=textureCube(reflectionCubeSampler,normalW,20.).rgb*vReflectionInfos.x;
environmentIrradiance=toLinearSpace(environmentIrradiance.rgb);
environmentIrradiance*=0.2; 
#endif
#else
vec2 coords=vReflectionUVW.xy;
#ifdef REFLECTIONMAP_PROJECTION
coords/=vReflectionUVW.z;
#endif
coords.y=1.0-coords.y;
#ifdef LODBASEDMICROSFURACE
environmentRadiance=texture2DLodEXT(reflection2DSampler,coords,lodReflection).rgb*vReflectionInfos.x;
#else
environmentRadiance=texture2D(reflection2DSampler,coords,biasReflection).rgb*vReflectionInfos.x;
#endif
environmentRadiance=toLinearSpace(environmentRadiance.rgb);
environmentIrradiance=texture2D(reflection2DSampler,coords,20.).rgb*vReflectionInfos.x;
environmentIrradiance=toLinearSpace(environmentIrradiance.rgb);
#endif
#endif
#ifdef OVERLOADEDVALUES
environmentIrradiance=mix(environmentIrradiance,vOverloadedReflection,vOverloadedMicroSurface.z);
environmentRadiance=mix(environmentRadiance,vOverloadedReflection,vOverloadedMicroSurface.z);
#endif
environmentRadiance*=vLightingIntensity.z;
environmentIrradiance*=vLightingIntensity.z;

vec3 specularEnvironmentReflectance=FresnelSchlickEnvironmentGGX(clamp(NdotV,0.,1.),specularEnvironmentR0,specularEnvironmentR90,sqrt(microSurface));

vec3 refractance=vec3(0.0,0.0,0.0);
#ifdef REFRACTION
vec3 transmission=vec3(1.0,1.0,1.0);
#ifdef LINKREFRACTIONTOTRANSPARENCY

transmission*=(1.0-alpha);


vec3 mixedAlbedo=surfaceAlbedoContribution.rgb*surfaceAlbedo.rgb;
float maxChannel=max(max(mixedAlbedo.r,mixedAlbedo.g),mixedAlbedo.b);
vec3 tint=clamp(maxChannel*mixedAlbedo,0.0,1.0);

surfaceAlbedoContribution*=alpha;

environmentIrradiance*=alpha;

surfaceRefractionColor*=tint;

alpha=1.0;
#endif

vec3 bounceSpecularEnvironmentReflectance=(2.0*specularEnvironmentReflectance)/(1.0+specularEnvironmentReflectance);
specularEnvironmentReflectance=mix(bounceSpecularEnvironmentReflectance,specularEnvironmentReflectance,alpha);

transmission*=1.0-specularEnvironmentReflectance;

refractance=surfaceRefractionColor*transmission;
#endif

surfaceAlbedo.rgb=(1.-reflectance)*surfaceAlbedo.rgb;
refractance*=vLightingIntensity.z;
environmentRadiance*=specularEnvironmentReflectance;

vec3 surfaceEmissiveColor=vEmissiveColor;
#ifdef EMISSIVE
vec3 emissiveColorTex=texture2D(emissiveSampler,vEmissiveUV+uvOffset).rgb;
surfaceEmissiveColor=toLinearSpace(emissiveColorTex.rgb)*surfaceEmissiveColor*vEmissiveInfos.y;
#endif
#ifdef OVERLOADEDVALUES
surfaceEmissiveColor=mix(surfaceEmissiveColor,vOverloadedEmissive,vOverloadedIntensity.w);
#endif
#ifdef EMISSIVEFRESNEL
float emissiveFresnelTerm=computeFresnelTerm(viewDirectionW,normalW,emissiveRightColor.a,emissiveLeftColor.a);
surfaceEmissiveColor*=emissiveLeftColor.rgb*(1.0-emissiveFresnelTerm)+emissiveFresnelTerm*emissiveRightColor.rgb;
#endif

#ifdef EMISSIVEASILLUMINATION
vec3 finalDiffuse=max(lightDiffuseContribution*surfaceAlbedoContribution+vAmbientColor,0.0)*surfaceAlbedo.rgb;
#ifdef OVERLOADEDSHADOWVALUES
shadowedOnlyLightDiffuseContribution=max(shadowedOnlyLightDiffuseContribution*surfaceAlbedoContribution+vAmbientColor,0.0)*surfaceAlbedo.rgb;
#endif
#else
#ifdef LINKEMISSIVEWITHALBEDO
vec3 finalDiffuse=max((lightDiffuseContribution+surfaceEmissiveColor)*surfaceAlbedoContribution+vAmbientColor,0.0)*surfaceAlbedo.rgb;
#ifdef OVERLOADEDSHADOWVALUES
shadowedOnlyLightDiffuseContribution=max((shadowedOnlyLightDiffuseContribution+surfaceEmissiveColor)*surfaceAlbedoContribution+vAmbientColor,0.0)*surfaceAlbedo.rgb;
#endif
#else
vec3 finalDiffuse=max(lightDiffuseContribution*surfaceAlbedoContribution+surfaceEmissiveColor+vAmbientColor,0.0)*surfaceAlbedo.rgb;
#ifdef OVERLOADEDSHADOWVALUES
shadowedOnlyLightDiffuseContribution=max(shadowedOnlyLightDiffuseContribution*surfaceAlbedoContribution+surfaceEmissiveColor+vAmbientColor,0.0)*surfaceAlbedo.rgb;
#endif
#endif
#endif
#ifdef OVERLOADEDSHADOWVALUES
finalDiffuse=mix(finalDiffuse,shadowedOnlyLightDiffuseContribution,(1.0-vOverloadedShadowIntensity.y));
#endif
#ifdef SPECULARTERM
vec3 finalSpecular=lightSpecularContribution*surfaceReflectivityColor;
#else
vec3 finalSpecular=vec3(0.0);
#endif
#ifdef SPECULAROVERALPHA
alpha=clamp(alpha+getLuminance(finalSpecular),0.,1.);
#endif
#ifdef RADIANCEOVERALPHA
alpha=clamp(alpha+getLuminance(environmentRadiance),0.,1.);
#endif


#ifdef EMISSIVEASILLUMINATION
vec4 finalColor=vec4(finalDiffuse*ambientColor*vLightingIntensity.x+surfaceAlbedo.rgb*environmentIrradiance+finalSpecular*vLightingIntensity.x+environmentRadiance+surfaceEmissiveColor*vLightingIntensity.y+refractance,alpha);
#else
vec4 finalColor=vec4(finalDiffuse*ambientColor*vLightingIntensity.x+surfaceAlbedo.rgb*environmentIrradiance+finalSpecular*vLightingIntensity.x+environmentRadiance+refractance,alpha);
#endif
#ifdef LIGHTMAP
#ifndef LIGHTMAPEXCLUDED
#ifdef USELIGHTMAPASSHADOWMAP
finalColor.rgb*=lightmapColor;
#else
finalColor.rgb+=lightmapColor;
#endif
#endif
#endif
finalColor=max(finalColor,0.0);
#ifdef CAMERATONEMAP
finalColor.rgb=toneMaps(finalColor.rgb);
#endif
finalColor.rgb=toGammaSpace(finalColor.rgb);
#ifdef LOGARITHMICDEPTH
gl_FragDepthEXT=log2(vFragmentDepth)*logarithmicDepthConstant*0.5;
#endif
#ifdef FOG
float fog=CalcFogFactor();
finalColor.rgb=fog*finalColor.rgb+(1.0-fog)*vFogColor;
#endif
#ifdef CAMERACONTRAST
finalColor=contrasts(finalColor);
#endif
finalColor.rgb=clamp(finalColor.rgb,0.,1.);
#ifdef CAMERACOLORGRADING
finalColor=colorGrades(finalColor);
#endif
#ifdef CAMERACOLORCURVES
finalColor.rgb=applyColorCurves(finalColor.rgb);
#endif



















gl_FragColor=finalColor;
}