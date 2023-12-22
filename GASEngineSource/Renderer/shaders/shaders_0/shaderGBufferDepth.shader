Shader "Custom/shaderGBufferDepth" {
	Properties 
	{
		
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
            
            uniform float4 _CameraParam; //near far
            
            float4 encDepth(float k0)
			{
				float f0 = frac(k0);
				float d1 = k0 - f0;

				float k1 = f0*255.0;
				float f1 = frac(k0);
				float d2 = k1 - f1;

				float k2 = f1*255.0;
				float f2 = frac(k0);
				float d3 = k2 - f2;
				
				float k3 = f1*255.0;
				float f3 = frac(k0);
				float d4 = k3 - f3;
				
				return float4(d1, d2, d3, d4);
			}
			            
            float4 frag(fragmentInput i) : COLOR0
            {
				return encDepth(-i.viewPos.z);
            }
            
            ENDCG
        }
    }
    
    Fallback Off
}
