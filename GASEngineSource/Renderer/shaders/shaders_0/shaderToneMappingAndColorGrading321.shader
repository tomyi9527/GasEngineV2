Shader "Custom/shaderToneMappingAndColorGrading321" 
{
	Properties 
	{
		_BaseTex ("Base (RGB)", 2D) = "white" {}
		_ColorGradingTex ("Base (RGB)", 2D) = "white" {}
		_LuminanceTex ("Base (RGB)", 2D) = "white" {}
	}

	SubShader 
	{
		Pass
		{	
			Blend Off
        	ZTest Off
        	ZWrite Off	
			CGPROGRAM
			#pragma target 3.0
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			uniform sampler2D _BaseTex;
			uniform sampler2D _ColorGradingTex;
			uniform sampler2D _LuminanceTex;
			uniform float4 RenderTargetResolution; //This is the render target resolution
			uniform float4 EyeAdaptParam; //float4(0.3f, 0.1f, 0, 0)
			uniform float4 HdrFinalParam1; //float4(0.03f, 0.26f, 5.0f, 1.04f)
			
			float3 openglColorGrading(sampler2D table, float3 src, float height)
			{	    
				//16*256 neutral LUT has a 17 stride
				float half_w = 0.5/(height*height);
				float half_h = 0.5/height;
				
				float v = 1.0f - src.g; //OpenGL Texture Mapping is different from DX V is 1.0 - V
				float v0 = src.r/height;
				
				float strideCount = height - 1.0f;
				float stride = 255.0f / strideCount;
				
				float k = (src.b*255.0f)/stride;
				float factor = frac(k);
				float k0 = k - factor;				
				float delta = 1.0f/height;					
				
				// x: 0 ~ 1.0 map to
				// k*delta + half_w ~ (k + 1)delta - half_w
				// x = [m - (k*delta + half_w)] / [(k + 1)delta - half_w - (k*delta + half_w)] 
				// x = [m - (k*delta + half_w)] / [delta - 2.0f*half_w]
				// m = x*[delta - 2.0f*half_w] + (k*delta + half_w)
				float u0_t = src.r*(delta - 2.0f*half_w) + (k0*delta + half_w);
				float u1_t = src.r*(delta - 2.0f*half_w) + ((k0 + 1.0f)*delta + half_w);
				
				// x: 0 ~ 1.0 map to
				// half_h ~ 1.0 - half_h
				// x = (m - half_h)/(1.0f - 2.0f*half_h)
				// m = (1.0f - 2.0f*half_h)*x + half_h;
				float v_t = (1.0f - 2.0f*half_h)*v + half_h;
				
				float3 color0 = tex2D(table, float2(u0_t, v_t)).rgb;
			    float3 color1 = tex2D(table, float2(u1_t, v_t)).rgb;
				
				return lerp(color0, color1, factor);
			}
						
			float3 DownScale(float3 color)
			{
				float exposureScale = 1.0f;//tex2D(ExposureSampler,0.5f);
				return color / exposureScale;
			}
			
			float3 ToneMap(float3 src)
			{
			    return 1.0f - exp2(-src);
			}
			
			float EyeAdaptionScale(float lum, float exposure)
			{
				float adaptLum = lerp(EyeAdaptParam.x, lum, EyeAdaptParam.y);
			    float scale = exposure/(1.e-6 + adaptLum);
				return scale;
			}

			float4 frag(v2f_img i) : COLOR
			{
				float2 biasedUV = i.uv;// + RenderTargetResolution.zw*0.5f;
				float4 sceneColor = tex2D(_BaseTex, biasedUV);
				//return sceneColor;
				sceneColor.rgb = DownScale(sceneColor.rgb);
				
				float4 adaptLum = tex2Dlod(_LuminanceTex, float4(0.5f, 0.5f, 0, 0));
				float scale = EyeAdaptionScale(adaptLum.x, HdrFinalParam1.w);
				
				sceneColor.xyz *= scale;
				
				sceneColor.rgb = ToneMap(sceneColor.rgb);
				sceneColor.rgb = openglColorGrading(_ColorGradingTex, sceneColor.rgb, 16.0f);
				
				float3 finalColor = pow(sceneColor, 1.0f/2.2f);
				
				float lumination = dot(finalColor.rgb, float3(0.2126f, 0.7152f, 0.0722f));

			    return float4(finalColor, lumination);
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
