Shader "Custom/shaderSceneLuminanceToOne319" 
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
			
			float4 frag(v2f_img i) : COLOR
			{			
				float result = 0.0f;    
			    result += tex2D(_DiffuseG, float2(0.25,0.25));
			    result += tex2D(_DiffuseG, float2(0.25,0.75));
			    result += tex2D(_DiffuseG, float2(0.75,0.25));
			    result += tex2D(_DiffuseG, float2(0.75,0.75));

			    result = exp(result/4);
			    return float4(result, result, result, 1.0f);
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
