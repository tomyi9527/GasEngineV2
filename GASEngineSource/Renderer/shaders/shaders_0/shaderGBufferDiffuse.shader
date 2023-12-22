Shader "Custom/shaderGBufferDiffuse" {
Properties 
	{
		_BaseTex ("Albedo (RGB)", 2D) = "white" {}
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
        	
        	Stencil
			{
                Ref			2
                ReadMask	2
                WriteMask	2
                Comp 		Always
                Pass 		Replace
                Fail		Keep
                ZFail 		Keep
            }
        	
        	ZTest Less
        	ZWrite On
        	
            CGPROGRAM
            
            #pragma vertex vert
            #pragma fragment frag
            
            #include "UnityCG.cginc"
            
            struct vertexInput 
            {
                float4 vertex : POSITION;
                float2 texcoord0 : TEXCOORD0;
                float3 normal :    NORMAL;
                float4 tangent :    TANGENT;
            };

            struct fragmentInput
            {
                float4 position : SV_POSITION;
                float2 texcoord0 : TEXCOORD0;
                float3 normal :    TEXCOORD1;
                float3 tangent :    TEXCOORD2;
                float3 binormal :   TEXCOORD3;
                float4 viewPos :   TEXCOORD4;
            };

            fragmentInput vert(vertexInput i)
            {
                fragmentInput o;
                o.position = mul(UNITY_MATRIX_MVP, i.vertex);
                o.texcoord0 = i.texcoord0;
                o.normal = mul((float3x3)_Object2World, i.normal.xyz);
                o.tangent = mul((float3x3)_Object2World, i.tangent.xyz);
                
                float3 binormal = cross(i.normal.xyz, i.tangent.xyz) * i.tangent.w;
                
                o.binormal = mul((float3x3)_Object2World, binormal.xyz);
                
                o.viewPos = mul(UNITY_MATRIX_MV, i.vertex);
                
                return o;
            }           
            
            uniform sampler2D _BaseTex : register(s0);
			            
            float4 frag(fragmentInput i) : COLOR0
            {
            	float4 diffuseColor = tex2D(_BaseTex, i.texcoord0);
            	clip(diffuseColor.a - 0.5f);
            	
            	return diffuseColor;
            }
            
            ENDCG
        }
    }
    
    Fallback Off
}
