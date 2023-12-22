Shader "Custom/shaderEyeball" 
{
	Properties 
	{
		_PupilColor ("Pupil Color", Color) = (1,1,1,1)
		_PupilSize ("Pupil Size", Range(0,2)) = 1.0
		//_SunLightDir ("Sun Light Direction", Vector) = (0, 0, 0, 0)
		//_SunColor ("Sun Color", Vector) = (0, 0, 0, 0)
		_SunLightDir ("Sun Light Direction", Vector) = (-0.234256, -0.4220845, 0.8757676, 0.0)
		_SunColor ("Sun Color", Vector) = (1.401961, 1.833333, 2.04902, 1.0)
		
		_SunScatterColor ("Sun Scatter Color", Vector) = (0.04235294, 0.04941177, 0.05882353, 1.0)
		_SkyLightColor ("Sky Light Color", Vector) = (0.9098039, 1.035294, 1.058824, 1.0)
		_GroundLightColor ("Ground Light Color", Vector) = (0.7215686, 0.8352941, 0.8235294, 1.0)
		//_SpecularIndirectLightingEnhance ("Specular Indirect Lighting Enhance", Vector) = (0.0, 0.0, 0.0, 0.0)
		//_SecondDirLightDir ("Second Light Direction", Vector) = (0, 0, 0, 0)
		//_SecondDirLightColor ("Second Light Color", Vector) = (0, 0, 0, 0)
		_SpecularIndirectLightingEnhance ("Specular Indirect Lighting Enhance", Vector) = (0.24, 0.0, 1.6, 0.385)
		_SecondDirLightDir ("Second Light Direction", Vector) = (0.0, 0.0, 1.0, 0.3)
		_SecondDirLightColor ("Second Light Color", Vector) = (0.2411765, 0.2509804, 0.2764706, 1.0)
			
		_BaseTex ("Base (RGB)", 2D) = "white" {}
		_NormalTex ("Normal (RGB)", 2D) = "white" {}
		_SpecularTex ("Specular (RGB)", 2D) = "white" {}
		_PupilBaseTex ("Pupil Base (RGB)", 2D) = "white" {}
		_PupilNormalTex ("Pupil Normal (RGB)", 2D) = "white" {}
	}
	
	SubShader 
	{
		Tags 
		{ 
			//"Queue" = "Geometry" 
		}
		
        Pass 
        {
        	Tags 
        	{ 
        		//"LightMode" = "Always" 
        	}
        	
        	Blend Off
        	ZTest LEqual
        	ZWrite On
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
            
            uniform float4 _PupilColor;
            uniform float _PupilSize;
            uniform float4 _SunLightDir;
            uniform float4 _SunColor;
            uniform float4 _SunScatterColor;
            uniform float4 _SkyLightColor;
            uniform float4 _GroundLightColor;
            uniform float4 _SpecularIndirectLightingEnhance;
            uniform float4 _SecondDirLightDir;
            uniform float4 _SecondDirLightColor;
 
            uniform sampler2D _BaseTex;
            uniform sampler2D _NormalTex;
            uniform sampler2D _SpecularTex;
            uniform sampler2D _PupilBaseTex;
            uniform sampler2D _PupilNormalTex;
            
            	#define EYEBALL_TEX_SIZE 512.0f
		#define PUPIL_TEX_SIZE 256.0f
		float2 CalcPupilUvScaling( float2 baseUv, float scalingFactor )
		{
			float2 pupilUv = baseUv - float2(0.5f, 0.5f);
			float sizeFactor = EYEBALL_TEX_SIZE / PUPIL_TEX_SIZE;
			pupilUv *= scalingFactor*sizeFactor;
			pupilUv += float2(0.5f, 0.5f);
			pupilUv.x = saturate(pupilUv.x);
			pupilUv.y = saturate(pupilUv.y);

			return pupilUv;
		}			
			
		float GetLuminance(float3 color)
		{
			const float3 lumWeight = float3(0.2125f, 0.7154f, 0.0721f);
			return dot(color, lumWeight);
		}

		float3 HslToRgb(float3 c)
		{
			float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
			float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
			return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
		}

		float3 CustomColor(float3 color, float h, float s, float l)
		{
			float3 hsl;
			hsl.r = h;
			hsl.g = s;
			hsl.b = l*GetLuminance(color);

			return HslToRgb(hsl);
		}
		
		#define GLOSS_ENCODE_FACTOR 13.0f
		float GetSpecPowerFromGloss(float gloss)
		{
		    return pow(2, GLOSS_ENCODE_FACTOR*gloss);
		}
		
		void EyePhyDirLighting(
					   //output
					   out float3 finalLighting, 
					   out float3 diffuseLighting, 
					   out float3 specLighting, 
					   out float3 adjustedDiffuse, 
					   out float ndl,
					   out float ndh,
					   //input lighting property
					   float3 lightDir, 
					   float3 lightColor,
					   float3 normal, 
					   float3 viewDir, 
					   float3 diffuseColor, 
					   float gloss, 
					   float3 specColor, 
					   float shadow)
		{
			//const preparation
			float3 h = normalize(-lightDir-viewDir);
			ndh = saturate(dot(normal, h));
			ndl = saturate(dot(-lightDir, normal));        	    
			float ndv = saturate(dot(-viewDir, normal));
			float vdh = saturate(dot(-viewDir, h));

			float3 spec = 0.0f;
			float specPower = GetSpecPowerFromGloss(gloss);	
			float invGloss = 1.0f-gloss;
			if(ndl > 0.0f)
			{
				float3 fresnelSpec = specColor;
				float distribute = pow(ndh, specPower);
				float geometry = (0.0397436*specPower+0.0856832f)/max(ndl, ndv);
				spec = fresnelSpec*distribute*geometry;
			}
			
			//diffuse calc
		    adjustedDiffuse = diffuseColor;
			float3 dif = ndl*adjustedDiffuse;

			//light calc
			float3 lighting = shadow*lightColor;
			
			//final
			finalLighting = (dif+spec)*lighting;	

			//this code will be optimized if not used
			diffuseLighting = dif*lighting;
			specLighting = spec*lighting;	
		}
		
		void QSAmbientLighting(
						    //outputs
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
			float lerpFactor = 0.5f + indirectEnhanceConstrast*0.5f;

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
		    float geometry = (0.0397436*specPower + 0.0856832f)*5.5f;	

		    specLighting = fresnelSpec*distribute*geometry*envColor+difLighting*gloss*fresnelComplement;
		    specLighting *= specularLightingMultiplier;

		    difLighting *= diffuse;
		}
		
		void ReadNormalGloss(sampler2D s, float2 uv, inout float3 normal, inout float gloss)
		{
		    float4 color = tex2D(s, uv);
		    float2 normalxy = (color.rg - 0.5f)*2.0f;//BiDecompress(color.rg);
		    float normalz = sqrt(1.0f-dot(normalxy,normalxy));
		    normal = float3(normalxy,normalz);
		    gloss = color.b;	
		}

		float4 frag(fragmentInput i) : COLOR0
		{
			float2 diffuseUv = i.texcoord0;
			float2 normalUv = i.texcoord0;
			float2 specularUv = i.texcoord0;

			float4 diffuseColor = tex2D(_BaseTex, diffuseUv);
			float2 pupilUv = CalcPupilUvScaling(diffuseUv, _PupilSize);

			float4 pupilDiffuse = tex2D(_PupilBaseTex, pupilUv);
			//pupilDiffuse.rgb = CustomColor(pupilDiffuse.rgb, _PupilColor.r, _PupilColor.g, _PupilColor.b);
			pupilDiffuse.rgb *= _PupilColor.rgb;

			diffuseColor.rgb = lerp(diffuseColor.rgb, pupilDiffuse.rgb, pupilDiffuse.a);
			//alpha test
			clip(diffuseColor.a - 0.5f);
			
			float3 specColor = tex2D(_SpecularTex, specularUv).xyz;

			float gloss;
			float3 normalEyeballBase;
			ReadNormalGloss(_NormalTex, normalUv, normalEyeballBase, gloss);
			normalEyeballBase.g = -normalEyeballBase.g;
			float3 normalEyeballBaseWS = mul(normalEyeballBase, float3x3(i.tangent, i.binormal, i.normal));    
		    normalEyeballBaseWS = normalize(normalEyeballBaseWS);
			
			float glossPupil;
			float3 normalPupil; 
			ReadNormalGloss(_PupilNormalTex, pupilUv, normalPupil, glossPupil);
			normalPupil.g = -normalPupil.g;
			float3 normalPupilWS = mul(normalPupil, float3x3(i.tangent, i.binormal, i.normal));    
			normalPupilWS = normalize(normalPupilWS);
			
			float3 normalWorldSpace = lerp(normalEyeballBaseWS.rgb, normalPupilWS.rgb, pupilDiffuse.a);
			gloss = lerp(gloss, glossPupil, pupilDiffuse.a);
			
			//normalWorldSpace *= vFace>=0.0f ? -1.0f : 1.0f;
			//normalWorldSpace *= -1.0f;
		    
			 //direct lighting	
			float3 toCamera = i.worldPos.xyz - _WorldSpaceCameraPos.xyz;
		    float3 viewDir = normalize(toCamera);
		    
			diffuseColor.rgb = pow(diffuseColor.rgb, 2.2);
			specColor.rgb = pow(specColor.rgb, 2.2);
			
			//test
//			float4 mSunLightDir = float4(-0.234256f, -0.4220845f, 0.8757676f, 0.0f);
//	 		float4 mSunColor = float4(1.401961f, 1.833333f, 2.04902f, 1.0f);
//	 		float4 mSkyLightColor = float4(0.9098039f, 1.035294f, 1.058824f, 1.0f);
//			float4 mGroundLightColor = float4(0.7215686f, 0.8352941f, 0.8235294f, 1.0f);
//			float4 mSunScatterColor = float4(0.04235294f, 0.04941177f, 0.05882353f, 1.0f);
//			float4 mSpecularIndirectLightingEnhance = float4(0.24f, 0.0f, 1.6f, 0.385f);
//			float4 mSecondDirLightDir = float4(0.0f, 0.0f, 1.0f, 0.3f);
//			float4 mSecondDirLightColor = float4(0.2411765f, 0.2509804f, 0.2764706f, 1.0f);
			//	
			    	
		    float shadow = 1.0f;		    	
		    float3 firstLighting = 0.0f, diffuseLighting = 0.0f, specLighting = 0.0f, adjustDiffuse = 0.0f;
		    float ndl, ndh;
		    EyePhyDirLighting(
					//outputs
					firstLighting, 
					diffuseLighting, 
					specLighting, 
					adjustDiffuse, 
					ndl, 
					ndh,
					//inputs
					_SunLightDir.xyz, 
					_SunColor.xyz,
					normalWorldSpace, 
					viewDir.xyz, 
					diffuseColor.xyz, 
					gloss, 
					specColor.xyz, 
					shadow);


			float3 sndLightResult = 0.0f, sndDiffuseLighting = 0.0f, sndSpecLighting = 0.0f, sndAdjustDiffuse = 0.0f;
			float sndNdl, sndNdh;	
			EyePhyDirLighting(
						//outputs
						sndLightResult, 
						sndDiffuseLighting, 
						sndSpecLighting, 
						sndAdjustDiffuse, 
						sndNdl, 
						sndNdh,
						//inputs
						_SecondDirLightDir.xyz, 
						_SecondDirLightColor.xyz,
						normalWorldSpace, 
						viewDir.xyz, 
						diffuseColor.xyz, 
						gloss, 
						specColor.xyz * _SecondDirLightDir.w, 
						1.0f);	

			float3 envDiffuse, envSpec;
	    	QSAmbientLighting(
	    				//outputs
	        			envDiffuse, 
	        			envSpec, 
	        			//inputs
	        			_SunLightDir.xyz, 
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
		
			float3 finalLighting = firstLighting + sndLightResult + envDiffuse;	
		
			//return float4(pupilDiffuse.a, 0, 0, 1);
			return float4(finalLighting.xyz, diffuseColor.a);
		}            
	           
		ENDCG
	}
    }
    
    Fallback Off
}
