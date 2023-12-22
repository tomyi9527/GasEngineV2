Shader "Custom/shaderDownscaleAndClampColor307" 
{
	Properties 
	{
		_DiffuseG ("Base (RGB)", 2D) = "white" {}
	}

	SubShader 
	{
		Pass
		{		
			CGPROGRAM
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			uniform sampler2D _DiffuseG;	//The size of this texture is twice of that of render target		
			uniform float4 RenderTargetResolution; //This is the render target resolution
			
			float4 ClampedSceneColor(float2 uv)
			{
				float4 result = tex2D(_DiffuseG, uv);
				return clamp(result, 0.0f, 20.0f);//20.0f is from an experience-tested value for hdr scene render
			}
			
			#define NUM_SAMPLES 4 // 4x4 -> 2x2
			
			float4 frag(v2f_img i) : COLOR
			{			
				float4 sumColor = 0;
				
				float2 biasedUV = i.uv;// + 0.5*RenderTargetResolution.zw;
				float2 lt = biasedUV + 0.5*RenderTargetResolution.zw*float2(-1, -1);
				float2 rt = biasedUV + 0.5*RenderTargetResolution.zw*float2( 1, -1);
				float2 lb = biasedUV + 0.5*RenderTargetResolution.zw*float2(-1,  1);
				float2 rb = biasedUV + 0.5*RenderTargetResolution.zw*float2( 1,  1);
				
				sumColor += ClampedSceneColor(lt) * 0.25f;
				sumColor += ClampedSceneColor(rt) * 0.25f;
				sumColor += ClampedSceneColor(lb) * 0.25f;
				sumColor += ClampedSceneColor(rb) * 0.25f;
				
				return sumColor;
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
