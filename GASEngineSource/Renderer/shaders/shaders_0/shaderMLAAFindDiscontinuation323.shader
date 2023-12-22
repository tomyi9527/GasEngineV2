Shader "Custom/shaderMLAAFindDiscontinuation323" 
{
	Properties 
	{
		_BaseTex ("Base (RGB)", 2D) = "white" {}
		_Threshold ("Threshold", Range(0,2)) = 0.15
	}

	SubShader 
	{
		Pass
		{	
			Stencil
			{
                Ref			1
                ReadMask	1
                WriteMask	1
                Comp 		Always
                Pass 		Replace
                Fail		Keep
                ZFail 		Keep
            }
            
			Blend Off
        	ZTest Off
        	ZWrite Off
        		
			CGPROGRAM
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			uniform sampler2D _BaseTex;
			uniform float _Threshold;
			uniform float4 _BufferResolution;

			float4 frag(v2f_img i) : COLOR
			{			
				float P = tex2D(_BaseTex, i.uv).a; //current lumin
				
				float2 L_UV = float2(i.uv.x - _BufferResolution.z, i.uv.y);
    			float L = tex2D(_BaseTex, L_UV).a;
    			
    			float2 T_UV = float2(i.uv.x, i.uv.y + _BufferResolution.w); //OpenGL texture coordination mapping 1 : 0
    			float T = tex2D(_BaseTex, T_UV).a;

    			float4 delta = abs(P.xxxx - float4(L, T, P.x, P.x));
    			float4 edges = step(_Threshold, delta); //search for edge
				clip(dot(edges, 1.0) - 0.001f);

    			return edges;//x horizonal discontinuity for vertial search
    						//y for horizontal search
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
