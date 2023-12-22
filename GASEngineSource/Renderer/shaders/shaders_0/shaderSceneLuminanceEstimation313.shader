Shader "Custom/shaderSceneLuminanceEstimation313" 
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

			uniform sampler2D _DiffuseG;
			uniform float4 RenderTargetResolution; //This is the render target resolution
			//render target 64*64 abgr16f
			float GetLuminance(float3 color)
			{
			    const float3 lumWeight = float3(0.2125f, 0.7154f, 0.0721f);
			    return dot(color, lumWeight);
			}

			float4 frag(v2f_img i) : COLOR
			{
			    float3 srcColor = 0.0f;
			    float logLumSum = 0.0f;
				float2 uv = i.uv;// + RenderTargetResolution.zw*0.5f; //uv bias

				for(float x = -1.0f; x <=1.0f; x += 1.0f)
				{
					for(float y = -1.0f; y <= 1.0f; y += 1.0f)
					{
						srcColor = tex2D(_DiffuseG, uv + float2(x,y)*RenderTargetResolution.zw);        
						float lum = GetLuminance(srcColor);
						logLumSum += log(lum + 1e-6);//1e-6防止log0
					}
				}
			        
			    logLumSum /= 9;

			    return float4(logLumSum, logLumSum, logLumSum, 1.0f);
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
