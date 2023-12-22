Shader "Custom/shaderGBuffer" 
{
	Properties 
	{
		_Color ("Color", Color) = (1,1,1,1)
		_BaseTex ("Diffuse (RGB)", 2D) = "white" {}
		_NormalTex ("Normal (RGB)", 2D) = "white" {}
		_SpecularTex ("Specular (RGB)", 2D) = "white" {}
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
            
            uniform sampler2D _BaseTex;
            uniform sampler2D _NormalTex;
            uniform sampler2D _SpecularTex;
            
            uniform float4 _CameraParam; //near far
            
            void PackGBufferWithGloss(out float4 difRt, out float4 normRt, out float4 specRt, float4 diffuse, float3 normal, float3 specColor, float gloss)
			{
			    difRt = diffuse;
			    float3 normalCompact = normal * 0.5f + 0.5f;
			    //normRt = float4(normalCompact.x, normalCompact.z, normalCompact.y, gloss); //QS coordination
			    normRt = float4(normalCompact, gloss); // unity coordination
			    specRt = float4(specColor, 1.0f); // original 0.0f
			}
			
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
            
            struct fragmentOutput
            {
                float4 c0 : COLOR0;
                float4 c1 : COLOR1;
                float4 c2 : COLOR2;
                float4 c3 : COLOR3;
            };

            fragmentOutput frag(fragmentInput i) 
            {
            	fragmentOutput output;
            	
            	//PackGBufferWithGloss(output.c0, output.c1, output.c2, 1, 1, 1, 1);
            	//output.c3 = 1;
            	//return output;            	
            	
            	//read diffuse
            	float4 diffuseColor = tex2D(_BaseTex, i.texcoord0);
            	clip(diffuseColor.a - 0.5f);

            	// read normal and gloss
            	float4 normalColor = tex2D(_NormalTex, i.texcoord0);
			    float2 normalXY = (normalColor.rg - 0.5f) * 2.0f;
			    float normalZ = sqrt(1.0f - dot(normalXY, normalXY));
			    float3 texSpaceNormal = normalize(float3(normalXY, normalZ));
			    float gloss = normalColor.b;
			    
			   	float3 normalWorldSpace = mul(texSpaceNormal, float3x3(i.tangent, i.binormal, i.normal)); 
				normalWorldSpace = normalize(normalWorldSpace);
            	
            	// read specColor
            	float3 specColor = tex2D(_SpecularTex, i.texcoord0).xyz;				
            	
               	PackGBufferWithGloss(output.c0, output.c1, output.c2, diffuseColor, normalWorldSpace, specColor, gloss);
               	//depth
               	//float k = i.position.z/i.position.w;
               	//float f = _ProjectionParams.y;
               	//float n = _ProjectionParams.z;
               	//float viewZ = (f*n)/(f-k*(f-n)); //< maybe wrong on OpenGL, because of the different project matrix
                //output.c3 = -(i.viewPos.z / 100.0f);//TODO: hardcode camera far plane
                //output.c3 = EncodeFloatRGBA(-i.viewPos.z);
                output.c3 = encDepth(-i.viewPos.z);
 
                return output;
            }
            
            ENDCG
        }
    }
    
    Fallback Off
}
