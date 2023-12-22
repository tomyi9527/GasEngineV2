Shader "Custom/shaderSkinLightingSpecular281" 
{
	Properties 
	{
		_DepthSampler ("Base (RGB)", 2D) = "white" {}
		_SrcSampler ("Base (RGB)", 2D) = "white" {}		
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
		#pragma target 3.0
		#pragma vertex vert_img
		#pragma fragment frag
		#include "UnityCG.cginc"
		
		//Specular caculation related
		//float4x4 ScreenToWorldTransform;
		//float3	 cameraPos;
		float4 _CameraParam; //r, t, n, f
		float4 _CameraRight;
		float4 _CameraUp;
		float4 _CameraForward;

		float4 SunLightDir;
		float4 SunColor;		//xyz is sun color, w is dir light fresnel
		float4 SkyLightColor;
		float4 GroundLightColor;
		float4 SunScatterColor;//xyz is positive color, w is fresnel factor
		float4 SpecularIndirectLightingEnhance;//x: indirect gloss multiplier, y: indirect specular lighting multiplier z: sky lighting cover factor w:ground lighting cover factor
		float4 SecondDirLightDir;
		float4 SecondDirLightColor;

		uniform sampler2D _SkinSSSDiffuse;
		uniform sampler2D _DiffuseG;
		uniform sampler2D _NormalGlossG;
		uniform sampler2D _SpecularG;
		
		#define GLOSS_ENCODE_FACTOR 13.0f
		
		void KelemanKalosPhyDirLighting(
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
									    float3 viewDir, 
									    float3 diffuseColor, 
									    float gloss, 
									    float3 specColor, 
									    float fresnelFactor, 
									    float shadow)
		{
		    //const preparation
		    float3 h = normalize(-lightDir-viewDir);
		    ndh = saturate(dot(normal, h));
		    ndl = saturate(dot(-lightDir, normal));        	    
		    float ndv = saturate(dot(-viewDir, normal));
		    float vdh = saturate(dot(-viewDir, h));

		    float3 spec = 0.0f;
		    float invGloss = 1.0f - gloss; //gloss 0~0.95f
			
			//fresnel
			float freBase = 1.0f - vdh;
			float freExp = pow(freBase, 5.0);
			float3 freResult = freExp+specColor*(1.0f - freExp);

			//distribution
			float roughness = (1.0f - gloss);	//gloss光泽度，越高roughness越低 	
			float invRoughSqr = 1.0f/(roughness*roughness);
			float alpha = acos(ndh); //half 和 n 之间的弧度值
			float ta = tan(alpha);  //将弧度值拉伸分布到高斯分布函数的全定义域
			float distribution = invRoughSqr/pow(ndh,4.0)*exp(-(ta*ta)*invRoughSqr); 
			spec = max(distribution*freResult/dot(h,h),0.0f)*ndl;			

		    //diffuse calc
		    adjustedDiffuse = diffuseColor*invGloss;
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
		    float specPower = pow(2,GLOSS_ENCODE_FACTOR*gloss);//GetSpecPowerFromGloss(gloss);  
		    float distribute = pow(ndh, specPower);
		    float geometry = (0.0397436*specPower+0.0856832f)*5.5f;	

		    specLighting = fresnelSpec*distribute*geometry*envColor+difLighting*gloss*fresnelComplement;
		    specLighting *= specularLightingMultiplier;

		    difLighting *= diffuse;
		}
		
		float3 calcViewVectorInWorldSpace(float2 uv)
		{
			float2 scaleUV = (uv - 0.5f)*2.0f;
			//scaleUV.y = -scaleUV.y;
			
			float3 farPos = float3(scaleUV*_CameraParam.xy, _CameraParam.w);
			float3 viewVector = normalize(farPos);
			float3x3 viewInv;
			viewInv[0] = _CameraRight.xyz;
			viewInv[1] = _CameraUp.xyz;
			viewInv[2] = _CameraForward.xyz;
			float3 worldVector = mul(viewVector, viewInv);
			return worldVector.xyz;
			
		}
		
		float4 frag(v2f_img i) : COLOR
		{
			// read normal + gloss
			float4 src = tex2D(_NormalGlossG, i.uv);	
		    float3 normal = src.xyz*2.0f - 1.0f;
		    float gloss = src.w; //0.0f~1.0f

			// viewDir In World Space
		    float3 viewDir = calcViewVectorInWorldSpace(i.uv);

			// diffuse + specular
			float4 specColor = tex2D(_SpecularG, i.uv);
			clip(specColor.a - 0.5f);
			specColor.rgb = pow(specColor.rgb, 2.2f);
			
			float4 diffuse = tex2D(_DiffuseG, i.uv);
			diffuse.xyz = pow(diffuse.xyz, 2.2f);
			
			//float shadow = diffuse.a;
			float shadow = 1.0f;

		    //MainLight lighting
			float fresnelFactor = SunColor.w;
			float directLightGloss = min(gloss*SpecularIndirectLightingEnhance.z, 0.95f);// x ~ 0.95f  SpecularIndirectLightingEnhance.z = 1.6f
			
			//skin			 
			float3 mainLightSpecularAndDiffuseLighting, mainLightDiffuseLighting, mainLightSpecularLighting, mainAdjustDiffuse;
			float mainNDL, mainNDH;
			KelemanKalosPhyDirLighting(
										//outputs
										mainLightSpecularAndDiffuseLighting, 
										mainLightDiffuseLighting, 
										mainLightSpecularLighting, 
										mainAdjustDiffuse, 
										mainNDL, 
										mainNDH,
										//inputs
										SunLightDir.xyz, 
										SunColor.xyz,
										normal, 
										viewDir.xyz, 
										diffuse.xyz, 
										directLightGloss, 
										specColor.xyz, 
										fresnelFactor, 
										shadow);	


			float3 secondLightSpecularAndDiffuseLighting, secondLightDiffuseLighting, secondLightSpecularLighting, secondAdjustDiffuse;		
			float secondNDL, secondNDH;
			KelemanKalosPhyDirLighting(
										//outputs
										secondLightSpecularAndDiffuseLighting, 
										secondLightDiffuseLighting, 
										secondLightSpecularLighting, 
										secondAdjustDiffuse, 
										secondNDL, 
										secondNDH,
										//inputs
										SecondDirLightDir.xyz, 
										SecondDirLightColor.xyz,
										normal, 
										viewDir.xyz, 
										diffuse.xyz, 
										directLightGloss, 
										specColor.xyz*SecondDirLightDir.w, 
										1.0f, 
										1.0f);	

			float3 totalSpecularLightingResult = mainLightSpecularLighting + secondLightSpecularLighting;
			
		    totalSpecularLightingResult = totalSpecularLightingResult.xyz * SpecularIndirectLightingEnhance.w; 

		    float3 ambientDiffuseLighting, ambientSpecularLighting;
			
		    QSAmbientLighting(
					    	//outputs
					        ambientDiffuseLighting, 
					        ambientSpecularLighting,
					        //inputs
					        SunLightDir.xyz, 
					        SunScatterColor.xyz, 
					        SkyLightColor.xyz, 
					        GroundLightColor.xyz, 
					        normal, 
					        viewDir, 
					        mainAdjustDiffuse, 
					        gloss, 
					        specColor.xyz, 
					        SpecularIndirectLightingEnhance, 
					        mainNDL, 
					        mainNDH);

			totalSpecularLightingResult += ambientSpecularLighting;
			float4 sssDiffuse = tex2D(_SkinSSSDiffuse, i.uv);
			
			totalSpecularLightingResult = max(totalSpecularLightingResult, 0.0f);//totalSpecularLightingResult have negative value because of the lighting calc
    		return float4(totalSpecularLightingResult + sssDiffuse.xyz, 1.0f);
    		//return float4(totalSpecularLightingResult, 1.0f);
//    		if(mainLightSpecularLighting.b <= 0.0)
//    			return float4(1, 1, 1, 1.0f);
//    		else
//    			return float4(0, 0, 0, 1.0f);
		}

		ENDCG
		} 
	}
}
