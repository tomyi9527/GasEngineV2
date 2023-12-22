Shader "Custom/shaderHair" 
{
	Properties 
	{			
		_TransFactor ("Trans Factor", Vector) = (2.0, 0.0, 0.0, 0.0)
		_SunLightDir ("Sun Light Direction", Vector) = (-0.234256, -0.4220845, 0.8757676, 0.0)
		_SunColor ("Sun Color", Vector) = (2.870588, 2.858824, 2.988235, 1.0)
		_SkyLightColor ("Sky Light Color", Vector) = (2.094118, 2.752941, 2.988235, 1.0)
		_GroundLightColor ("Ground Light Color", Vector) = (0.05490196, 0.05098039, 0.07843138, 1.0)	
		_SunScatterColor ("Sun Scatter Color", Vector) = (0.04823529, 0.05019608, 0.06078431, 1.0)		
		_SpecularIndirectLightingEnhance ("Specular Indirect Lighting Enhance", Vector) = (0.24, 0.0, 0, 0)
		_SpecularShiftFactor ("Specular Shift Factor", Range(0,1)) = 0.5
		_SecondarySpecularShiftFactor ("Secondary Specular Shift Factor", Range(0,1)) = 0.5
		_SecondaryGlossFactor ("Secondary Gloss Factor", Range(0,1)) = 1.0
		_SecondarySpecularColor ("Secondary Specular Color", Vector) = (0.4, 0.4, 0.4, 0.0)
		_HairShadowThreshold ("Hair Shadow Threshold", Vector) = (0.15, 0.0, 0.0, 0.0)
			
		_BaseTex ("Base (RGB)", 2D) = "white" {}
		_NormalTex ("Normal (RGB)", 2D) = "white" {}
		_SpecularTex ("Specular (RGB)", 2D) = "white" {}
	}
	
	SubShader 
	{
		Tags 
		{ 
			//"Queue" = "Transparent" 
		}
		
        Pass 
        {
        	Tags 
        	{ 
        		//"LightMode" = "Always" 
        	}
        	
        	Blend SrcAlpha OneMinusSrcAlpha, Zero One
        	BlendOp Add
        	ZTest LEqual
        	ZWrite Off
        	//Cull Off
        	
            CGPROGRAM
            
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 3.0
            #include "UnityCG.cginc"
            
            struct vertexInput 
            {
                float4 vertex : POSITION;
                float2 texcoord0 : TEXCOORD0;
                float3 normal :    NORMAL;
                float4 tangent :    TANGENT;
            };

            struct fragmentInput
            {
                float4 position : SV_POSITION;
                float2 texcoord0 : TEXCOORD0;
                float3 normal :    TEXCOORD1;
                float3 tangent :    TEXCOORD2;
                float3 binormal :   TEXCOORD3;
                float4 worldPos :   TEXCOORD4;
            };
            
            float4x4 _MATRIX_MVP;

            fragmentInput vert(vertexInput i)
            {
                fragmentInput o;
                o.position = mul(UNITY_MATRIX_MVP, i.vertex);
                o.texcoord0 = i.texcoord0;
                o.normal = mul((float3x3)_Object2World, i.normal.xyz);
                o.tangent = mul((float3x3)_Object2World, i.tangent.xyz);
                
                float3 binormal = cross(i.normal.xyz, i.tangent.xyz) * i.tangent.w;
                
                o.binormal = mul((float3x3)_Object2World, binormal.xyz);
                
                o.worldPos = mul(_Object2World, i.vertex);
                
                return o;
            }
            
            uniform float4 _TransFactor;
            uniform float4 _SunLightDir;
            uniform float4 _SunColor;            
            uniform float4 _SkyLightColor;
            uniform float4 _GroundLightColor;
            uniform float4 _SunScatterColor;
            uniform float4 _SpecularIndirectLightingEnhance;
            uniform float _SpecularShiftFactor;
			uniform float _SecondarySpecularShiftFactor;
			uniform float _SecondaryGlossFactor;
			uniform float4 _SecondarySpecularColor;
			uniform float4 _HairShadowThreshold;//x min threshold
 
            uniform sampler2D _BaseTex;
            uniform sampler2D _NormalTex;
            uniform sampler2D _SpecularTex;
			
			#define GLOSS_ENCODE_FACTOR 13.0f
			float GetSpecPowerFromGloss(float gloss)
			{
			    return pow(2,GLOSS_ENCODE_FACTOR*gloss);
			}

			float KajiyaKaySpec(float3 t, float3 h, float specPower)
			{
				float tdh = dot(t, h);
			    return pow(sqrt(max(1.0f-tdh*tdh,0.01f)), specPower);	
			}

			void KajiyaKayPhyDirLighting(
			    //outputs
			    out float3 finalLighting, 
			    out float3 diffuseLighting, 
			    out float3 specLighting, 
			    out float3 adjustedDiffuse, 
			    out float ndl, 
			    out float ndh,
			    //inputs
			    float3 lightDir, 
			    float3 lightColor, 
			    float3 normal, 
			    float3 tangent, 
			    float3 viewDir, 
			    float3 diffuseColor, 
			    float gloss, 
			    float3 specColor, 
			    float fresnelFactor, 
			    float shadow, 
				float3 tangentSec, 
				float glossSec, 
				float3 specColorSec)
			{	    			    
			    //const preparation
			    float3 h = normalize(-lightDir-viewDir);
			    ndh = saturate(dot(normal, h));
			    ndl = saturate(dot(-lightDir, normal));        	    
			    float ndv = saturate(dot(-viewDir, normal));
			    float vdh = saturate(dot(-viewDir, h));

			    float3 spec = 0.0f;
			    float invGloss = 1.0f-gloss;
			    float tdh = dot(tangent, h);
			    spec = KajiyaKaySpec(tangent, h, GetSpecPowerFromGloss(gloss))*specColor*ndl;	
				spec += KajiyaKaySpec(tangentSec, h, GetSpecPowerFromGloss(glossSec))*specColorSec*ndl;	

			    //diffuse calc
			    adjustedDiffuse = diffuseColor*0.4f;
			    float3 dif = max(0.0f, 0.75f*ndl + 0.25f)*adjustedDiffuse;

			    //light calc
			    float3 lighting = shadow*lightColor;

			    //final
			    finalLighting = (dif+spec)*lighting;

			    //this code will be optimized if not used
			    diffuseLighting = dif*lighting;
			    specLighting = spec*lighting;	
			}
			
			void QSAmbientLighting(
    			//output
			    out float3 difLighting, 
			    out float3 specLighting,
			    //inputs
			    float3 sunScaterDir, 
			    float3 sunScaterColor, 
			    float3 skyLightColor, 
			    float3 groundLightColor,
			    float3 normal, 
			    float3 viewDir, 
			    float3 diffuse, 
			    float gloss, 
			    float3 specColor, 
			    float4 indirectSpecEnhance, 
			    float ndl, 
			    float ndh)
			{
				float indirectEnhanceConstrast = indirectSpecEnhance.y;	
				float lerpFactor = 0.5f+indirectEnhanceConstrast*0.5f;

				float sunScaterLight = max(lerp(1.0f, dot(normal, -sunScaterDir), lerpFactor), 0.0f);   
				difLighting = sunScaterLight*sunScaterColor;

				//sky hemisphere light lighting
				float skyLight = max(lerp(1.0f, normal.z, lerpFactor), 0.0f);    
				difLighting += skyLight*skyLightColor;

				//ground light, enhanced ground light color    
				float groundLight = max(lerp(0.75f, -normal.z, lerpFactor), 0.0f);
				difLighting += groundLightColor*groundLight;    	

			    //--specular part of indirect lighting--    
				float glossMultiplier = indirectSpecEnhance.x;
			    float specularLightingMultiplier = 2.0f;

			    float3 reflectVec = reflect(viewDir, normal);
			    float3 envColor = max(dot(reflectVec, -sunScaterDir)*0.75f+0.25f,0.0f)*sunScaterColor;

			    gloss = min(gloss*glossMultiplier,0.95f);
			    float edh = saturate(dot(-viewDir, normal)); 
			    float invGloss = 1.0f-gloss;
			    float invFresFactor = 1.0f/(1.0f+15.0f*invGloss);
			    float3 fresnelComplement = (1.0f-specColor)*pow(1.0f-edh, 5.0f)*invFresFactor;
			    float3 fresnelSpec = specColor+fresnelComplement;    

				float3 h = (-sunScaterDir-viewDir)*0.5f;
				ndh = saturate(dot(normal, h));
			    float specPower = GetSpecPowerFromGloss(gloss);  
			    float distribute = pow(ndh, specPower);
			    float geometry = (0.0397436*specPower+0.0856832f)*5.5f;	

			    specLighting = fresnelSpec*distribute*geometry*envColor+difLighting*gloss*fresnelComplement;
			    specLighting *= specularLightingMultiplier;

			    difLighting *= diffuse;
			}

			float4 frag(fragmentInput i) : COLOR0
			{
				float2 diffuseUv = i.texcoord0;
				float2 normalUv = i.texcoord0;
				float2 specularUv = i.texcoord0;

			    float4 diffuseColor = tex2D(_BaseTex, diffuseUv);
				float shiftAmount = 1.0f;

				float4 specColorMap = tex2D(_SpecularTex, specularUv);
				float3 specColor = specColorMap.xyz;  		
				shiftAmount = specColorMap.w;
				
				float specIntensity = 0.0f;
				
				float4 normColor = tex2D(_NormalTex, normalUv);

				float2 normalxy = (normColor.rg - 0.5f)*2.0f;//BiDecompress(normColor.rg);
				float normalz = sqrt(1.0f-dot(normalxy,normalxy));
				float3 normalTexSpace = float3(normalxy,normalz);

				float gloss = normColor.b;	

			    float3 normalWorldSpace = mul(normalTexSpace, float3x3(i.tangent, i.binormal, i.normal));
			    normalWorldSpace = normalize(normalWorldSpace);	
			    
				float3 toCamera = i.worldPos.xyz - _WorldSpaceCameraPos.xyz;
		    	float3 viewDir = normalize(toCamera);
		    
				diffuseColor.rgb = pow(diffuseColor.rgb,2.2);
				specColor.rgb = pow(specColor.rgb,2.2);

				float shadow = 1.0;					

				float3 sunLighting = 0.0f, diffuseLighting = 0.0f, specLighting = 0.0f, adjustDiffuse = 0.0f;
				float ndl, ndh;	
				float3 specTangent = normalize(i.tangent + normalWorldSpace*(shiftAmount-0.5f)*_SpecularShiftFactor);
				float3 specTangentSec = normalize(i.tangent - normalWorldSpace*_SecondarySpecularShiftFactor);
				float glossSec = gloss*_SecondaryGlossFactor;
				float3 specColorSec = specColor*_SecondarySpecularColor.xyz;
				
				float3 finalLighting = 0.0f;

			    KajiyaKayPhyDirLighting(
					//outputs
					sunLighting, 
					diffuseLighting, 
					specLighting, 
					adjustDiffuse, 
					ndl, 
					ndh,
					//inputs
					_SunLightDir.xyz, 
					_SunColor.xyz,
					normalWorldSpace, 
					specTangent, 
					viewDir.xyz, 
					diffuseColor.xyz, 
					gloss, 
					specColor, 
					_SunColor.w, 
					shadow, 
					specTangentSec, 
					glossSec, 
					specColorSec);
					
				finalLighting += sunLighting;	

				float3 skyKajiyaSpec;
				float3 skyResult;
				float skyNdl,skyNdh;
				KajiyaKayPhyDirLighting(
					//outputs
					skyResult, 
					diffuseLighting, 
					specLighting, 
					adjustDiffuse, 
					skyNdl, 
					skyNdh,
					//inputs
					float3(-_SunLightDir.x, -_SunLightDir.y, _SunLightDir.z),
					_SkyLightColor.xyz,					
					normalWorldSpace, 
					specTangent, 
					viewDir.xyz, 
					diffuseColor.xyz, 
					gloss, 
					specColor, 
					_SunColor.w, 
					1.0f, 
					specTangentSec, 
					glossSec, 
					specColorSec);
				
				finalLighting += skyResult;
			
				float3 envDiffuse, envSpec;
			    QSAmbientLighting(
			    	//outputs
			        envDiffuse, 
			        envSpec,
			        //inputs
			        _SunLightDir, 
			        _SunScatterColor.xyz, 
			        _SkyLightColor.xyz, 
			        _GroundLightColor.xyz, 
			        normalWorldSpace, 
			        viewDir, 
			        adjustDiffuse, 
			        gloss, 
			        specColor, 
			        _SpecularIndirectLightingEnhance, 
			        ndl, 
			        ndh);
						
				finalLighting += envDiffuse;		
				
				return float4(finalLighting.xyz, saturate(diffuseColor.w*_TransFactor.x));
			}            
		           
			ENDCG
		}
    }
    
	Fallback Off
}
