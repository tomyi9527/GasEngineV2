Shader "Custom/shaderDownscaleHalf312" 
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
				
			float4 frag(v2f_img i) : COLOR
			{			
				float4 sumColor = 0;
				
				float2 biasedUV = i.uv;// + 0.5*RenderTargetResolution.zw;
				float2 lt = biasedUV + 0.5*RenderTargetResolution.zw*float2(-1, -1);
				float2 rt = biasedUV + 0.5*RenderTargetResolution.zw*float2( 1, -1);
				float2 lb = biasedUV + 0.5*RenderTargetResolution.zw*float2(-1,  1);
				float2 rb = biasedUV + 0.5*RenderTargetResolution.zw*float2( 1,  1);
				
				sumColor += tex2D(_DiffuseG, lt) * 0.25f;
				sumColor += tex2D(_DiffuseG, rt) * 0.25f;
				sumColor += tex2D(_DiffuseG, lb) * 0.25f;
				sumColor += tex2D(_DiffuseG, rb) * 0.25f;
				
				return float4(sumColor.xyz, 1.0f);;
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
