Shader "Custom/shaderGBufferNormal" {
Properties 
	{
		_NormalTex ("Normal (RGB)", 2D) = "white" {}
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
       	
        	ZTest Equal
        	ZWrite Off
        	
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
            
            uniform sampler2D _NormalTex : register(s1);
			            
            float4 frag(fragmentInput i) : COLOR0
            {            	
				float4 normalColor = tex2D(_NormalTex, i.texcoord0);
			    float2 normalXY = (normalColor.rg - 0.5f) * 2.0f;
			    float normalZ = sqrt(1.0f - dot(normalXY, normalXY));
			    float3 texSpaceNormal = normalize(float3(normalXY, normalZ));
			    float gloss = normalColor.b;
			    
			    float3 normalWorldSpace = mul(texSpaceNormal, float3x3(i.tangent, i.binormal, i.normal)); 
				normalWorldSpace = normalize(normalWorldSpace);
				
				float3 normalCompact = normalWorldSpace * 0.5f + 0.5f;
			    return float4(normalCompact, gloss); // unity coordination
            }
            
            ENDCG
        }
    }
    
    Fallback Off
}
