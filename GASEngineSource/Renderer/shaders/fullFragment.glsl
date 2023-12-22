#define PI 3.1415926535897932384626433832795
#define PI_2 (2.0*3.1415926535897932384626433832795)
#define INV_PI 1.0/PI
#define INV_LOG2 1.4426950408889634073599246810019
#define DefaultGamma 2.4
#define LUV


#ifdef ALBEDO_MAP
	uniform sampler2D albedoMap;
#endif
	uniform vec3 albedoColor;
	uniform float albedoFactor;

#ifdef OUTPUT_DEPTH_ONLY
	uniform vec2 cameraNearFar;
#endif

#ifdef NORMAL_MAP
	uniform sampler2D normalMap;
	uniform float normalFactor;
	uniform int normalFlipY;
#endif


#ifdef CLASSIC_SHADING
	uniform vec3 specularColor;
	uniform sampler2D specularMap;
	uniform float specularFactor;
	
	uniform sampler2D glossinessMap;
	uniform float glossinessFactor;
#else

	#ifdef ELECTRIC
		#ifdef SPECULAR_MAP
			uniform sampler2D specularMap;
		#endif
		uniform vec3 specularColor;		
		uniform float specularFactor;
	#else
		#ifdef METALNESS_MAP
			uniform sampler2D metalnessMap;
		#endif
			uniform float metalnessFactor;

		#ifdef FRESNEL_MAP
			uniform sampler2D fresnelMap;
		#endif
			uniform float fresnelFactor;	
	
		#ifdef ROUGHNESS_MAP
			uniform sampler2D roughnessMap;
		#endif
			uniform float roughnessFactor;

		#ifdef AOMAP
			uniform sampler2D aoMap;
			uniform float aoFactor;
			uniform int aoOccludeSpecular;
		#endif

		#ifdef CAVITY_MAP
			uniform sampler2D cavityMap;
			uniform float cavityFactor;
		#endif
	#endif

#endif


#ifdef EMISSIVEMAP
	uniform sampler2D emissiveMap;
	uniform vec3 emissiveColor;
	uniform float emissiveFactor;
	//uniform int emissiveMultiplicative;
#endif

#ifdef TRANSPARENCY
	uniform sampler2D transparencyMap;
	uniform float transparencyFactor;
	uniform int transparencyAlphaInvert;
#endif

#ifdef CUBEMAP
		uniform samplerCube environmentSpecularCube;
#else
	#ifdef PANORAMA
		uniform sampler2D environmentSpecularPanorama;
	#endif
#endif

uniform vec2 uEnvironmentLodRange;
uniform vec2 uEnvironmentSpecularTextureSize;
uniform vec3 uEnvironmentSphericalHarmonics[9];
uniform mat4 uEnvironmentTransform;
uniform sampler2D uIntegrateBRDF;

uniform float uEnvironmentExposure;
uniform float uRGBMRange;
uniform int outputLinear;

varying vec4 fragmentColor;
varying vec2 fragmentTexCoord0;
varying vec2 fragmentTexCoord1;
varying vec3 fragmentNormal;
varying vec4 fragmentTangent;
varying vec3 fragmentViewPosition;


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
float sRGBToLinear(const in float c) 
{
    return SRGB_LIN(c);
}
vec3 sRGBToLinear(const in vec3 c) 
{
    return vec3(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b));
}
vec4 sRGBToLinear(const in vec4 c) 
{
    return vec4(SRGB_LIN(c.r), SRGB_LIN(c.g), SRGB_LIN(c.b), c.a);
}

//http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec3 RGBMToRGB( const in vec4 rgba ) 
{
    const float maxRange = 8.0;
    return rgba.rgb * maxRange * rgba.a;
}

const mat3 LUVInverse = mat3( 6.0013,    -2.700,   -1.7995,
                              -1.332,    3.1029,   -5.7720,
                              0.3007,    -1.088,    5.6268 );

vec3 LUVToRGB( const in vec4 vLogLuv ) 
{
    float Le = vLogLuv.z * 255.0 + vLogLuv.w;
    vec3 Xp_Y_XYZp;
    Xp_Y_XYZp.y = exp2((Le - 127.0) / 2.0);
    Xp_Y_XYZp.z = Xp_Y_XYZp.y / vLogLuv.y;
    Xp_Y_XYZp.x = vLogLuv.x * Xp_Y_XYZp.z;
    vec3 vRGB = LUVInverse * Xp_Y_XYZp;
    return max(vRGB, 0.0);
}

// http://graphicrants.blogspot.fr/2009/04/rgbm-color-encoding.html
vec4 encodeRGBM(const in vec3 col, const in float range) 
{
	//Convert the colors from linear to gamma space before encoding.
	if (range <= 0.0)
	{
		return vec4(col, 1.0);
	}

    vec4 rgbm;
    vec3 color = col / range;
    rgbm.a = clamp( max( max( color.r, color.g ), max( color.b, 1e-6 ) ), 0.0, 1.0 );
    rgbm.a = ceil( rgbm.a * 255.0 ) / 255.0;
    rgbm.rgb = color / rgbm.a;
    return rgbm;
}

vec3 decodeRGBM(const in vec4 col, const in float range) 
{
	if (range <= 0.0)
	{
		return col.rgb;
	}

    return range * col.rgb * col.a;
}

vec3 textureRGB(const in sampler2D texture, const in vec2 uv) 
{
    return texture2D(texture, uv.xy ).rgb;
}

vec4 textureRGBA(const in sampler2D texture, const in vec2 uv) 
{
    return texture2D(texture, uv.xy ).rgba;
}

float textureIntensity(const in sampler2D texture, const in vec2 uv) 
{
    return texture2D(texture, uv).r;
}

float textureAlpha(const in sampler2D texture, const in vec2 uv) 
{
    return texture2D(texture, uv.xy ).a;
}

vec3 mtexNspaceTangent(const in vec4 tangent, const in vec3 normal, const in vec3 texnormal) {
	vec3 tang = vec3(0.0, 1.0, 0.0);
	float l = length(tangent.xyz);
	if (l != 0.0) {
		//normalize reusing length computations
		// tang =  normalize(tangent.xyz);
		tang = tangent.xyz / l;
	}
	vec3 B = tangent.w * normalize(cross(normal, tang));
	return normalize(texnormal.x*tang + texnormal.y*B + texnormal.z*normal);
}

vec2 normalMatcap(const in vec3 normal, const in vec3 nm_z) {
	vec3 nm_x = vec3(-nm_z.z, 0.0, nm_z.x);
	vec3 nm_y = cross(nm_x, nm_z);
	return vec2(dot(normal.xz, nm_x.xz), dot(normal, nm_y)) * vec2(0.5) + vec2(0.5); //MADD vector form
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
		vec3 tang = tangent.xyz / l;
		vec3 binormal = tangent.w * normalize(cross(normal, tang));
		outnormal = normal + gradient.x * tang + gradient.y * binormal;
	}
	else {
		outnormal = vec3(normal.x + gradient.x, normal.y + gradient.y, normal.z);
	}
	return normalize(outnormal);
}

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

vec3 evaluateDiffuseSphericalHarmonics( const in vec3 s[9], const in mat3 envTrans, const in vec3 N )
{
    vec3 n = envTrans * N;
    // https://github.com/cedricpinson/envtools/blob/master/Cubemap.cpp#L523
    vec3 result = (s[0]+s[1]*n.y+s[2]*n.z+s[3]*n.x+s[4]*n.y*n.x+s[5]*n.y*n.z+s[6]*(3.0*n.z*n.z-1.0)+s[7]*(n.z*n.x)+s[8]*(n.x*n.x-n.y*n.y));
    return max(result, vec3(0.0));
}

// Frostbite, Lagarde paper p67
// http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr.pdf
float linRoughnessToMipmap( const in float roughnessLinear ) 
{
    return sqrt(roughnessLinear);
}

vec3 integrateBRDF( const in vec3 specular, const in float r, const in float NoV, const in sampler2D tex ) 
{
	vec4 rgba = texture2D(tex, vec2(NoV, r));
	float b = (rgba[3] * 65280.0 + rgba[2] * 255.0);
	float a = (rgba[1] * 65280.0 + rgba[0] * 255.0);
	const float div = 1.0 / 65535.0;

	return (specular * a + b) * div;
}

// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
// TODO should we use somehow specular f0 ?
vec3 integrateBRDFApprox( const in vec3 specular, const in float roughness, const in float NoV ) 
{
    const vec4 c0 = vec4( -1, -0.0275, -0.572, 0.022 );
    const vec4 c1 = vec4( 1, 0.0425, 1.04, -0.04 );
    vec4 r = roughness * c0 + c1;
    float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
    vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
    return specular * AB.x + AB.y;
}

vec3 computeIBLDiffuseUE4( const in vec3 normal, const in vec3 albedo, const in mat3 envTrans, const in vec3 sphHarm[9] ) 
{
    return albedo * evaluateDiffuseSphericalHarmonics(sphHarm, envTrans, normal) ;
}

// basically whether it's panorama or cubemap we load the adequate glsl
// and we set samplerEnv and prefilterEnvMap
#ifdef CUBEMAP
// has to be before anything else, just after glsl language version
// it's require. enable just allow
vec3 textureCubemapLod(const in samplerCube texture, const in vec3 dir, const in float lod )
{
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

vec3 textureCubeLodEXTFixed(const in samplerCube texture, const in vec2 size, const in vec3 direction, const in float lodInput, const in float maxLod) {
	vec3 dir = direction;
	float lod = min(maxLod, lodInput);

	// http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
	float scale = 1.0 - exp2(lod) / size.x;
	vec3 absDir = abs(dir);
	float M = max(max(absDir.x, absDir.y), absDir.z);

	if (absDir.x != M) dir.x *= scale;
	if (absDir.y != M) dir.y *= scale;
	if (absDir.z != M) dir.z *= scale;

	return textureCubemapLod(texture, dir, lod);
}

vec3 prefilterEnvMapCube(const in float rLinear, const in vec3 R, const in samplerCube tex, const in vec2 lodRange, const in vec2 size) {
	float lod = linRoughnessToMipmap(rLinear) * lodRange[1]; //( uEnvironmentMaxLod - 1.0 );
	return textureCubeLodEXTFixed(tex, size, R, lod, lodRange[0]);
}

#define samplerEnv samplerCube
#define prefilterEnvMap prefilterEnvMapCube

#else
	#ifdef PANORAMA
		//vec2 computeUVForMipmap(const in float level, const in vec2 uv, const in float size, const in float maxLOD)
		//{
		//	// width for level
		//	float widthForLevel = exp2(maxLOD - level);
		//	// the height locally for the level in pixel
		//	// to opimitize a bit we scale down the v by two in the inputs uv
		//	float heightForLevel = widthForLevel * 0.5;
		//	// compact version
		//	float texelSize = 1.0 / size;
		//	vec2 uvSpaceLocal = vec2(1.0) + uv * vec2(widthForLevel - 2.0, heightForLevel - 2.0);
		//	uvSpaceLocal.y += size - widthForLevel;
		//	return uvSpaceLocal * texelSize;
		//}

		vec2 computeUVForMipmap(const in float level, const in vec2 uv, const in float size, const in float maxLOD) 
		{
			// width for level
			float widthForLevel = exp2(maxLOD - level);
			// the height locally for the level in pixel
			// to opimitize a bit we scale down the v by two in the inputs uv
			float heightForLevel = widthForLevel * 0.5;

			#if 0
				float texelSize = 1.0 / size;

				float resizeX = (widthForLevel - 2.0) * texelSize;
				float resizeY = (heightForLevel - 2.0) * texelSize;

				float uvSpaceLocalX = texelSize + uv.x * resizeX;
				float uvSpaceLocalY = texelSize + uv.y * resizeY;

				uvSpaceLocalY += (size - widthForLevel) / size;

				return vec2(uvSpaceLocalX, uvSpaceLocalY);
			#else
				// compact version
				float texelSize = 1.0 / size;
				vec2 uvSpaceLocal = vec2(1.0) + uv * vec2(widthForLevel - 2.0, heightForLevel - 2.0);
				uvSpaceLocal.y += size - widthForLevel;
				return uvSpaceLocal * texelSize;
			#endif
		}

		//for y up
		vec2 normalToPanoramaUVY(const in vec3 dir)
		{
			float n = length(dir.xz);

			// to avoid bleeding the max(-1.0,dir.x / n) is needed
			vec2 pos = vec2((n>0.0000001) ? max(-1.0, dir.x / n) : 0.0, dir.y);

			// fix edge bleeding
			if (pos.x > 0.0)
			{
				pos.x = min(0.999999, pos.x);
			}

			pos = acos(pos) * INV_PI;

			pos.x = (dir.z > 0.0) ? pos.x*0.5 : 1.0 - (pos.x*0.5);

			// shift u to center the panorama to -z
			pos.x = mod(pos.x - 0.25 + 1.0, 1.0);
			pos.y = 1.0 - pos.y;

			return pos;
		}

		// for z up
		vec2 normalToPanoramaUVZ(const in vec3 dir)
		{
			float n = length(dir.xy);

			// to avoid bleeding the max(-1.0,dir.x / n) is needed
			vec2 pos = vec2((n>0.0000001) ? max(-1.0, dir.x / n) : 0.0, dir.z);

			// fix edge bleeding
			if (pos.x > 0.0)
			{
				pos.x = min(0.999999, pos.x);
			}

			pos = acos(pos)*INV_PI;

			// to avoid bleeding the limit must be set to 0.4999999 instead of 0.5
			pos.x = (dir.y > 0.0) ? pos.x*0.5 : 1.0 - (pos.x*0.5);

			// shift u to center the panorama to -y
			pos.x = mod(pos.x - 0.25 + 1.0, 1.0);
			pos.y = 1.0 - pos.y;

			return pos;
		}

		#define normalToPanoramaUV normalToPanoramaUVY

		vec3 texturePanorama(const in sampler2D texture, const in vec2 uv)
		{
			vec4 rgba = texture2D(texture, uv);
		#ifdef FLOAT
			return rgba.rgb;
		#endif

		#ifdef RGBM
			return RGBMToRGB(rgba);
		#endif

		#ifdef LUV
			return LUVToRGB(rgba);
		#endif
		}

		vec3 texturePanoramaLod(const in sampler2D texture, const in vec2 size, const in vec3 direction, const in float lodInput, const in float maxLOD)
		{
			float lod = min(maxLOD, lodInput);
			vec2 uvBase = normalToPanoramaUV(direction);

			// we scale down v here because it avoid to do twice in sub functions
			// uvBase.y *= 0.5;

			float lod0 = floor(lod);
			vec2 uv0 = computeUVForMipmap(lod0, uvBase, size.x, maxLOD);
			vec3 texel0 = texturePanorama(texture, uv0.xy);
			float lod1 = ceil(lod);
			vec2 uv1 = computeUVForMipmap(lod1, uvBase, size.x, maxLOD);
			vec3 texel1 = texturePanorama(texture, uv1.xy);

			return mix(texel0, texel1, fract(lod));
		}

		vec3 prefilterEnvMapPanorama(const in float rLinear, const in vec3 R, const in sampler2D tex, const in vec2 lodRange, const in vec2 size)
		{
			return texturePanoramaLod(tex, size, R, linRoughnessToMipmap(rLinear) * lodRange[1], lodRange[0]);
		}

		#define samplerEnv sampler2D
		#define prefilterEnvMap prefilterEnvMapPanorama

	#else
		#define samplerEnv sampler2D
		vec3 prefilterEnvMap( const in float rLinear, const in vec3 R, const in sampler2D tex, const in vec2 lodRange, const in vec2 size )
		{
			return vec3(0.0);
		}		
	#endif // PANORAMA

#endif // CUBEMAP

vec3 getSpecularDominantDir( const in vec3 N, const in vec3 R, const in float realRoughness ) 
{
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
	) {
#else
	, const in sampler2D texBRDF) {
#endif

	float rough = max(rLinear, 0.0);

	//float NoV = clamp(dot(N, V), 0.0, 1.0);
	float NoV = dot(N, V);
	vec3 R = normalize(NoV * 2.0 * N - V);
	R = getSpecularDominantDir(N, R, rLinear);
	// could use that, especially if NoV comes from shared preCompSpec
	//vec3 R = reflect(-V, N);

	vec3 prefilteredColor = prefilterEnvMap(rough, envTrans * R, texEnv, lodRange, size);
	
	// http://marmosetco.tumblr.com/post/81245981087
	// marmoset uses 1.3, we force it to 1.0
	float factor = clamp(1.0 + dot(R, frontNormal), 0.0, 1.0);
	prefilteredColor *= factor * factor;
	
#ifdef MOBILE
	return prefilteredColor * integrateBRDFApprox(specular, rough, NoV);
#else
	//modified by tomyi
	//float clampedNoV = clamp(NoV, 0.0, 1.0);
	return prefilteredColor * integrateBRDF(specular, rough, NoV, texBRDF);
#endif
}

#ifdef OUTPUT_DEPTH_ONLY
	vec4 encodeDepth(float viewZ)
	{
		float depth = (viewZ * 1.0 - cameraNearFar.x) / (cameraNearFar.y - cameraNearFar.x);
		vec4 tmp = fract(vec4(1.0, 255.0, 65025.0, 16581375.0) * depth);
		tmp -= tmp.yzww * vec4(1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0, 0.0);
		tmp.a = 1.0;

		return tmp;
	}
#endif

#ifdef CLASSIC_SHADING
	float adjustSpecular(const in float specular, const in vec3 normal) 
	{
		// Based on The Order : 1886 SIGGRAPH course notes implementation (page 21 notes)
		float normalLen = length(normal);
		if(normalLen < 1.0) 
		{
			float normalLen2 = normalLen*normalLen;
			float kappa = (3.0*normalLen -  normalLen2*normalLen)/(1.0 - normalLen2);
			// http://www.frostbite.com/2014/11/moving-frostbite-to-pbr/
			// page 91 : they use 0.5/kappa instead
			return 1.0 - min(1.0, sqrt((1.0-specular)*(1.0-specular) + 1.0/kappa));
		}

		return specular;
	}

	vec3 cubemapSeamlessFixDirection(const in vec3 direction, const in float size ) 
	{
		vec3 dir = direction;
		float scale = 1.0 - 1.0/size;
		// http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
		vec3 absDir = abs(dir);
		float M = max(max(absDir.x, absDir.y), absDir.z);

		if(absDir.x != M) dir.x *= scale;
		if(absDir.y != M) dir.y *= scale;
		if(absDir.z != M) dir.z *= scale;

		return dir;
	}

	// seamless cubemap for background ( no lod )
	vec3 textureCubeFixed(const in samplerCube tex, const in vec3 direction, const in float scale ) 
	{
		// http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
		vec3 dir = cubemapSeamlessFixDirection(direction, scale);
		return LUVToRGB(textureCube(tex, dir));
	}

	vec3 textureEnvironmentCube( const in samplerCube tex, const in mat3 envTrans, const in vec3 N, const in vec3 V, const in vec2 size )
	{
		vec3 R = normalize((2.0*clamp(dot(N, V), 0.0, 1.0))*N - V);
		return textureCubeFixed(tex, envTrans*R, size[0]);
	}

	vec3 textureEnvironmentCubeWithGlossiness(const in samplerCube texEnv,
												const in float glossiness,
												const in mat3 envTrans, 
												const in vec3 N, 
												const in vec3 V,
												const in vec2 lodRange, 
												const in vec2 size)
	{
		float roughness = 1.0 - glossiness;
		roughness = max(1.e-4, roughness*0.99999);

		//float NoV = clamp(dot(N, V), 0.0, 1.0);
		float NoV = dot(N, V);
		vec3 R = normalize(NoV * 2.0 * N - V);
		R = getSpecularDominantDir(N, R, roughness);
		// could use that, especially if NoV comes from shared preCompSpec
		//vec3 R = reflect(-V, N);

		vec3 prefilteredColor = prefilterEnvMap(roughness, envTrans*R, texEnv, lodRange, size);

		return prefilteredColor;
	}

#endif

void main() 
{
	gl_FragColor = vec4(1.0, 0, 0, 1.0);
	return;
}