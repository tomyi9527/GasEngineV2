Shader "Custom/shaderLuminanceAdaption320" 
{
	Properties 
	{
		_CalcLuminance ("Base (RGB)", 2D) = "white" {}
		_CurrentLuminance ("Base (RGB)", 2D) = "white" {}
	}

	SubShader 
	{
		Pass
		{		
			CGPROGRAM
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			uniform sampler2D _CalcLuminance;	//1x1
			uniform sampler2D _CurrentLuminance;	//1x1
			uniform float AdaptSpeed; // 0.0165f
			
			float4 frag(v2f_img i) : COLOR
			{			
				float adaptLum = tex2D(_CalcLuminance, 0.5f).r;
			    float currentLum = tex2D(_CurrentLuminance, 0.5f).r;           	
				float result = lerp(adaptLum, currentLum, AdaptSpeed);
			    result = max(result,0.0f);
			    
			    return result;
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
