Shader "Custom/shaderSky" 
{
	Properties 
	{
		_BaseTex ("Albedo (RGB)", 2D) = "white" {}
		_DiffuseColor ("Diffuse Color", Color) = (0.3215686, 0.3215686, 0.3215686, 1)
		_FxHighlightFactor ("Fx Highlight Factor", Vector) = (6.0, 5.717647, 5.670588, 1)
	}

	SubShader 
	{
		Pass
		{
		
		ZTest Off
        ZWrite Off
          	
		CGPROGRAM
		#pragma vertex vert_img
		#pragma fragment frag
		#include "UnityCG.cginc"

		uniform sampler2D _BaseTex;		
		uniform float4 _DiffuseColor;
		uniform float4 _FxHighlightFactor;
		
		float4 frag(v2f_img i) : COLOR
		{			
			//return float4(tex2D(_BaseTex, i.uv).xyz, 1);
    			float4 diffuseColor = _DiffuseColor;
				diffuseColor *= tex2D(_BaseTex, i.uv);
				diffuseColor.rgb = pow(diffuseColor.rgb, 2.2);
				diffuseColor.rgb *= dot(_FxHighlightFactor.xyz, float3(0.2125f, 0.7154f, 0.0721f));
				return diffuseColor;
		}

		ENDCG
		} 
	}
}
