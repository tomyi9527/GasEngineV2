Shader "Custom/shaderFaceSkin" 
{
	Properties 
	{
		_Color ("Color", Color) = (1,1,1,1)
		_BaseTex ("Albedo (RGB)", 2D) = "white" {}
		_NormalTex ("Albedo (RGB)", 2D) = "white" {}
		_SpecularTex ("Albedo (RGB)", 2D) = "white" {}
		_Glossiness ("Smoothness", Range(0,1)) = 0.5
		_Metallic ("Metallic", Range(0,1)) = 0.0
	}
	
	SubShader 
	{
		Tags 
		{ 
			"Queue" = "Geometry" 
		}
		
        Pass 
        {
        	Tags 
        	{ 
        		"LightMode" = "Always" 
        	}
            CGPROGRAM
            
            #pragma vertex vert
            #pragma fragment frag
            
            #include "UnityCG.cginc"
            
            struct vertexInput 
            {
                float4 vertex : POSITION;
                float4 texcoord0 : TEXCOORD0;
                float3 normal :    TEXCOORD1;
                float3 tangent :    TEXCOORD2;
                float3 binormal :   TEXCOORD3;
            };

            struct fragmentInput
            {
                float4 position : SV_POSITION;
                float4 texcoord0 : TEXCOORD0;
                float3 normal :    TEXCOORD1;
                float3 tangent :    TEXCOORD2;
                float3 binormal :   TEXCOORD3;
            };

            fragmentInput vert(vertexInput i)
            {
                fragmentInput o;
                o.position = mul(UNITY_MATRIX_MVP, i.vertex);
                o.texcoord0 = i.texcoord0;
                o.normal = i.normal;
                o.tangent = i.tangent;
                o.binormal = i.binormal;
                return o;
            }           
            
            uniform sampler2D _BaseTex;
            uniform sampler2D _NormalTex;
            uniform sampler2D _SpecularTex;

            fixed4 frag(fragmentInput i) : SV_Target 
            {
                 return tex2D(_BaseTex, i.texcoord0);
            }
            
            ENDCG
        }
    }
    
    Fallback Off
}
