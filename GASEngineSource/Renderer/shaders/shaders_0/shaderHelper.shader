Shader "Custom/shaderHelper" 
{
	Properties 
	{
		_ColorGradingTex ("Base (RGB)", 2D) = "white" {}
		_AreaCalcTex ("Base (RGB)", 2D) = "white" {}
	}

	SubShader 
	{
		Pass
		{		
			CGPROGRAM
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			uniform sampler2D _ColorGradingTex;
			uniform sampler2D _AreaCalcTex;
				
			float4 frag(v2f_img i) : COLOR
			{	
				//return float4(i.uv.x, i.uv.y, 0, 1.0f);
				float4 c0 =  float4(tex2D(_AreaCalcTex, i.uv).rgb, 1.0f);
				float4 c1 =  float4(tex2D(_ColorGradingTex, i.uv).rgb, 1.0f);
				return c0*c1;
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
