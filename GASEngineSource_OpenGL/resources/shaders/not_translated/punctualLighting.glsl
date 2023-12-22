
//HEMISPHERE LIGHTS
#if HEMISPHERE_LIGHT_COUNT > 0

struct HemisphereLight 
{
	vec3 direction;
	vec3 skyColor;
	vec3 groundColor;
};

uniform HemisphereLight hemisphereLights[HEMISPHERE_LIGHT_COUNT];

#endif

//SPOT LIGHTS
#if SPOT_LIGHT_COUNT > 0

struct SpotLight 
{
	vec3 position;
	vec3 direction;
	vec3 color;
	float distance;
	float decay;
	float coneCos;
	float penumbraCos;	
};

uniform SpotLight spotLights[SPOT_LIGHT_COUNT];

#endif

//POINT LIGHTS
#if POINT_LIGHT_COUNT > 0

struct PointLight 
{
	vec3 position;
	vec3 color;
	float distance;
	float decay;
	int shadow;
};

uniform PointLight pointLights[POINT_LIGHT_COUNT];

#endif

//DIRECTIONAL LIGHTS
#if DIRECTIONAL_LIGHT_COUNT > 0

struct DirectionalLight 
{
	vec3 direction;
	vec3 color;
};

uniform DirectionalLight directionalLights[DIRECTIONAL_LIGHT_COUNT];

#endif


vec3 getHemisphereLightIrradiance(const in HemisphereLight hemiLight, const in GeometricContext geometry) 
{
	float dotNL = dot(geometry.normal, hemiLight.direction);
	float hemiDiffuseWeight = 0.5 * dotNL + 0.5;
	vec3 irradiance = mix(hemiLight.groundColor, hemiLight.skyColor, hemiDiffuseWeight);
#ifndef PHYSICALLY_CORRECT_LIGHTS
	irradiance *= PI;
#endif
	return irradiance;
}
#endif


#ifdef CLASSIC_SHADING

	float getLightAttenuation(const in float dist, const in vec4 lightAttenuation) 
	{
		// lightAttenuation(constantEnabled, linearEnabled, quadraticEnabled)
		// TODO find a vector alu instead of 4 scalar
		float constant = lightAttenuation.x;
		float linear = lightAttenuation.y*dist;
		float quadratic = lightAttenuation.z*dist*dist;
		return 1.0 / (constant + linear + quadratic);
	}

	void specularCookTorrance(
		const in vec3 n, 
		const in vec3 l, 
		const in vec3 v, 
		const in float hard, 
		const in vec3 materialSpecular, 
		const in vec3 lightSpecular, 
		out vec3 specularContrib) 
	{
		vec3 h = normalize(v + l);
		float nh = dot(n, h);
		float specfac = 0.0;

		if (nh > 0.0) 
		{
			float nv = max(dot(n, v), 0.0);
			float i = pow(nh, hard);
			i = i / (0.1 + nv);
			specfac = i;
		}
		// ugly way to fake an energy conservation (mainly to avoid super bright stuffs with low glossiness)
		float att = hard > 100.0 ? 1.0 : smoothstep(0.0, 1.0, hard * 0.01);
		specularContrib = specfac*materialSpecular*lightSpecular*att;
	}

	void lambert(
		const in float ndl,
		const in vec3 materialDiffuse, 
		const in vec3 lightDiffuse, 
		out vec3 diffuseContrib) 
	{
		diffuseContrib = ndl*materialDiffuse*lightDiffuse;
	}

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
			eyeLightDir = dist > 0.0 ? eyeLightDir / dist : vec3(0.0, 1.0, 0.0);
			if (lightCosSpotCutoff > 0.0) 
			{
				//compute lightSpotBlend
				vec3 lightSpotDirectionEye = normalize(mat3(vec3(lightInvMatrix[0]), vec3(lightInvMatrix[1]), vec3(lightInvMatrix[2]))*lightSpotDirection);

				float cosCurAngle = dot(-eyeLightDir, lightSpotDirectionEye);
				float diffAngle = cosCurAngle - lightCosSpotCutoff;
				float spot = 1.0;
				if (diffAngle < 0.0) 
				{
					spot = 0.0;
				}
				else
				{
					if (lightSpotBlend > 0.0)
					{
						spot = cosCurAngle * smoothstep(0.0, 1.0, (cosCurAngle - lightCosSpotCutoff) / (lightSpotBlend));
					}
				}

				if (spot > 0.0) 
				{
					// compute NdL
					float NdotL = dot(eyeLightDir, normal);
					if (NdotL > 0.0) 
					{
						lighted = true;
						vec3 diffuseContrib;
						lambert(NdotL, materialDiffuse, lightDiffuse, diffuseContrib);
						vec3 specularContrib;
						specularCookTorrance(normal, eyeLightDir, eyeVector, materialShininess, materialSpecular, lightSpecular, specularContrib);
						return spot*attenuation*(diffuseContrib + specularContrib);
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
		out bool lighted) 
	{
		eyeLightPos = vec3(lightMatrix * lightPosition);
		eyeLightDir = eyeLightPos - vViewVertex.xyz;
		float dist = length(eyeLightDir);
		// compute dist
		// compute attenuation
		float attenuation = getLightAttenuation(dist, lightAttenuation);
		if (attenuation != 0.0) 
		{
			// compute direction
			eyeLightDir = dist > 0.0 ? eyeLightDir / dist : vec3(0.0, 1.0, 0.0);
			// compute NdL
			float NdotL = dot(eyeLightDir, normal);
			if (NdotL > 0.0) 
			{
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
		out bool lighted) 
	{
		lighted = false;
		eyeLightDir = normalize(vec3(lightMatrix*lightPosition));
		// compute NdL   // compute NdL
		float NdotL = dot(eyeLightDir, normal);
		if (NdotL > 0.0)
		{
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
		out bool lighted) 
	{
		lighted = false;

		eyeLightDir = normalize(vec3(lightMatrix * lightPosition));
		float NdotL = dot(eyeLightDir, normal);
		float weight = 0.5 * NdotL + 0.5;
		vec3 diffuseContrib = materialDiffuse * mix(lightGround, lightDiffuse, weight);

		// same cook-torrance as above for sky/ground
		float skyWeight = 0.5 * dot(normal, normalize(eyeVector + eyeLightDir)) + 0.5;
		float gndWeight = 0.5 * dot(normal, normalize(eyeVector - eyeLightDir)) + 0.5;
		float skySpec = pow(skyWeight, materialShininess);
		float skyGround = pow(gndWeight, materialShininess);
		float divisor = (0.1 + max(dot(normal, eyeVector), 0.0));
		float att = materialShininess > 100.0 ? 1.0 : smoothstep(0.0, 1.0, materialShininess * 0.01);
		vec3 specularContrib = lightDiffuse * materialSpecular * weight * att * (skySpec + skyGround) / divisor;

		return diffuseContrib + specularContrib;
	}

#else

	#define G1V(dotNV, k) (1./(dotNV*(1.-k)+k))

	vec4 LightingFuncPrep(const in vec3 N,
		const in vec3 V,
		const in float roughness)
	{

		float dotNV = saturate(dot(N, V));
		float alpha = roughness * roughness;
		float k = alpha * .5;
		float visNV = G1V(dotNV, k);

		vec4 prepSpec;

		prepSpec.x = alpha;
		prepSpec.y = alpha * alpha;
		prepSpec.z = k;
		prepSpec.w = visNV;

		return prepSpec;

	}

	vec3 LightingFuncUsePrepGGX(
		const vec4 prepSpec,
		const vec3 N,
		const vec3 V,
		const vec3 L,
		const vec3 F0,
		const float dotNL)
	{

		vec3 H = normalize(V + L);
		float dotNH = saturate(dot(N, H));
		// D
		float alphaSqr = prepSpec.y;
		float denom = dotNH * dotNH *(alphaSqr - 1.) + 1.;
		float D = alphaSqr / (PI * denom * denom);
		// F
		float dotLH = saturate(dot(L, H));
		float dotLH5 = pow(1. - dotLH, 5.);
		vec3 F = vec3(F0) + (vec3(1.) - F0)*(dotLH5);
		// V
		float visNL = G1V(dotNL, prepSpec.z);
		vec3 specular = D * F * visNL * prepSpec.w;
		return specular;
	}

	// pure compute Light PBR
	vec3 computeLight(
		const in vec3 lightColor,
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
			eyeLightDir = dist > 0.0 ? eyeLightDir / dist : vec3(0.0, 1.0, 0.0);
			if (lightCosSpotCutoff > 0.0)
			{
				//compute lightSpotBlend
				vec3 lightSpotDirectionEye = normalize(mat3(vec3(lightInvMatrix[0]), vec3(lightInvMatrix[1]), vec3(lightInvMatrix[2]))*lightSpotDirection);

				float cosCurAngle = dot(-eyeLightDir, lightSpotDirectionEye);
				float diffAngle = cosCurAngle - lightCosSpotCutoff;
				float spot = 1.0;
				if (diffAngle < 0.0) 
				{
					spot = 0.0;
				}
				else
				{
					if (lightSpotBlend > 0.0)
					{
						spot = cosCurAngle * smoothstep(0.0, 1.0, (cosCurAngle - lightCosSpotCutoff) / (lightSpotBlend));
					}
				}

				if (spot > 0.0)
				{
					// compute NdL
					float NdotL = dot(eyeLightDir, normal);
					if (NdotL > 0.0)
					{
						lighted = true;
						return spot*attenuation*computeLight(lightColor, albedo, normal, eyeVector, eyeLightDir, specular, prepSpec, NdotL);
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
		eyeLightPos = vec3(lightMatrix * lightPosition);
		eyeLightDir = eyeLightPos - vViewVertex.xyz;
		float dist = length(eyeLightDir);
		// compute dist
		// compute attenuation
		float attenuation = getLightAttenuation(dist, lightAttenuation);
		if (attenuation != 0.0)
		{
			// compute direction
			eyeLightDir = dist > 0.0 ? eyeLightDir / dist : vec3(0.0, 1.0, 0.0);
			// compute NdL
			float NdotL = dot(eyeLightDir, normal);
			if (NdotL > 0.0)
			{
				lighted = true;
				return  attenuation * computeLight(lightColor, albedo, normal, eyeVector, eyeLightDir, specular, prepSpec, NdotL);
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
		eyeLightDir = normalize(vec3(lightMatrix * lightPosition));
		// compute NdL
		float NdotL = dot(eyeLightDir, normal);
		if (NdotL > 0.0)
		{
			lighted = true;
			return computeLight(lightColor, albedo, normal, eyeVector, eyeLightDir, specular, prepSpec, NdotL);
		}
		return vec3(0.0);
	}
#endif