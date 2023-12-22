Shader "Custom/shaderLighting277" 
{
	Properties 
	{
		_DiffuseG ("Base (RGB)", 2D) = "white" {}
		_NormalGlossG ("Base (RGB)", 2D) = "white" {}
		_SpecularG ("Base (RGB)", 2D) = "white" {}
		
	}

	SubShader 
	{
		Pass
		{
		
			Stencil
			{
                Ref			2
                ReadMask	2
                WriteMask	2
                Comp 		Equal
                Pass 		Keep
                Fail		Keep
                ZFail 		Keep
            }
            
            ZTest Off
          	ZWrite Off
		
		CGPROGRAM
		#pragma vertex vert_img
		#pragma fragment frag
		#include "UnityCG.cginc"

		uniform sampler2D _DiffuseG;
		uniform sampler2D _NormalGlossG;
		uniform sampler2D _SpecularG;
		
		uniform float4 SunLightDir; //float4(-0.234256f, 0.8757676f, -0.4220845f, 0.0f)
		uniform float4 SunColor;//float4(1.401961f, 1.833333f, 2.04902f, 1.0f)		//xyz is sun color, w is dir light fresnel
		uniform float4 SkyLightColor;//float4(0.9098039f, 1.035294f, 1.058824f, 1.0f)
		uniform float4 GroundLightColor; //float4(0.7215686f, 0.8352941f, 0.8235294f, 1.0f)
		uniform float4 SunScatterColor; //float4(0.04235294f, 0.04941177f, 0.05882353f, 1.0f) //xyz is positive color, w is fresnel factor
		uniform float4 SpecularIndirectLightingEnhance;//float4(0.24f, 0.0f, 1.6f, 0.385f) //x: indirect gloss multiplier, y: indirect specular lighting multiplier z: sky lighting cover factor w:ground lighting cover factor
		uniform float4 SecondDirLightDir;//float4(0.0f, 1.0f, 0.0f, 0.3f)
		uniform float4 SecondDirLightColor;//float4(0.2411765f, 0.2509804f, 0.2764706f, 1.0f)
		
		#define INV_SSS_SCALE_FACTOR	0.25f
		
		void KelemanKalosPhyDirLighting(
										out float3 diffuseLighting, 
										out float3 adjustedDiffuse, 
										out float ndl,
										//inputs
										float3 lightDir, float3 lightColor, float3 normal, float3 diffuseColor, float gloss, float shadow)
		{    
		    ndl = saturate(dot(-lightDir, normal));
		    float invGloss = 1.0f-gloss;
		    //diffuse calc
		    adjustedDiffuse = diffuseColor*invGloss;
		    float3 dif = ndl*adjustedDiffuse;

		    //light calc
		    float3 lighting = shadow*lightColor;
		    diffuseLighting = dif*lighting;
		}
		
		void QSAmbientLighting(
							    out float3 difLighting,
							    //inputs
							    float3 sunScaterDir, float3 sunScaterColor, float3 skyLightColor, float3 groundLightColor,
							    float3 normal, float3 diffuse, float4 indirectSpecEnhance, float ndl)
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

		    difLighting *= diffuse;
		}
		
		float4 frag(v2f_img i) : COLOR
		{
			//read gbuffer data
			float4 src = tex2D(_NormalGlossG, i.uv);	
  			float3 normal = src.xyz*2.0f-1.0f;
  			float gloss = src.w;
  			
			float4 diffuse = tex2D(_DiffuseG, i.uv);
			diffuse.xyz = pow(diffuse.xyz, 2.2f);

			float shadow = 1.0f;
			//float shadow = diffuse.a;

  			//direct lighting
  			float3 diffuseLighting, adjustDiffuse;	
			float directLightGloss = gloss;
			float ndl;

			diffuse.xyz = INV_SSS_SCALE_FACTOR;//reduce diffuse to pure color, to collect pure lighting info, use 0.25 as we use 7e3 buffer, unable to capture so high range lighting info

			KelemanKalosPhyDirLighting(
									//outputs
									diffuseLighting, adjustDiffuse, ndl,
									//inputs
									SunLightDir.xyz, SunColor.xyz, normal, diffuse.xyz, directLightGloss, shadow);	


			float3 sndDiffuseLighting, sndAdjustDiffuse;		
			float sndNdl;
			KelemanKalosPhyDirLighting(
									//outputs
									sndDiffuseLighting, sndAdjustDiffuse, sndNdl,
									//inputs
									SecondDirLightDir.xyz, SecondDirLightColor.xyz, normal, diffuse.xyz, directLightGloss, 1.0f);

			diffuseLighting += sndDiffuseLighting;	
			float3 result = diffuseLighting.xyz;
    		float3 ambientDiffuse, ambientSpec, envSpec = 0.0f;
	
    		QSAmbientLighting(
    						//input
        					ambientDiffuse,
        					//output
        					SunLightDir.xyz, SunScatterColor.xyz, SkyLightColor.xyz, GroundLightColor.xyz, 
        					normal, adjustDiffuse, SpecularIndirectLightingEnhance, ndl);
        					
			result += ambientDiffuse;
    		return float4(result, 1.0f);
		}

		ENDCG
		} 
	}
}
