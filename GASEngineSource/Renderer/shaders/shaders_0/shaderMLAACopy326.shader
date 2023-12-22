Shader "Custom/shaderMLAACopy326" 
{
	Properties 
	{
		_BaseTex ("Albedo (RGB)", 2D) = "white" {}
	}

	SubShader 
	{
		Pass
		{		
			CGPROGRAM
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			uniform sampler2D _BaseTex;		
			
			float4 frag(v2f_img i) : COLOR
			{
				float4 diffuseColor = tex2D(_BaseTex, i.uv);
				return diffuseColor;
			}

			ENDCG
		} 
	}
}
