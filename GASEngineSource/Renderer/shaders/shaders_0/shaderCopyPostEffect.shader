Shader "Custom/shaderCopyPostEffect" 
{
	Properties
	{
		_SourceTex ("", 2D) = "white" {}
	}
 
	SubShader 
	{ 
		ZTest Always 
		Cull Off 
		ZWrite Off 
		Fog { Mode Off } //Rendering settings
 
		Pass
		{
			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
			#include "UnityCG.cginc" 
		  //we include "UnityCG.cginc" to use the appdata_img struct
    
			struct v2f
			{
				float4 pos : POSITION;
				half2 uv : TEXCOORD0;
			};
   
			//Our Vertex Shader 
			v2f vert (appdata_img v)
			{
				v2f o;
				o.pos = mul (UNITY_MATRIX_MVP, v.vertex);
				#if UNITY_UV_STARTS_AT_TOP
					o.uv = float2(v.texcoord.x, 1 - v.texcoord.y);
				# else
					o.uv = float2(v.texcoord.x, v.texcoord.y);
				# endif
				return o; 
			}
			    
    		uniform sampler2D _SourceTex;
    		
			float4 frag(v2f i) : COLOR
			{
				float4 orgCol = tex2D(_SourceTex, i.uv);
				//float avg = (orgCol.r + orgCol.g + orgCol.b)/3.0f;
				//fixed4 col = fixed4(avg, avg, avg, 1);
     
				return orgCol;
			}
			ENDCG
		}
	} 
}
