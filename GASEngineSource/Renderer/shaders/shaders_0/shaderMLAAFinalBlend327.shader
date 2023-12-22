Shader "Custom/shaderMLAAFinalBlend327" 
{
	Properties 
	{
		_BaseTex ("Base (RGB)", 2D) = "white" {}
		_WeightTex ("Base (RGB)", 2D) = "white" {}
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
                Comp 		Equal
                Pass 		Keep
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
			uniform sampler2D _WeightTex;
			uniform float4 _BufferResolution;

			float4 frag(v2f_img i) : COLOR
			{			
				//return float4(1, 0, 0, 1);
				float4 offset = i.uv.xyxy + _BufferResolution.zwzw * float4(1.0, 0.0, 0.0, -1.0);  //OPENGL w = -w
				
				//r: left g:asright b:top a:bottom
				// Fetch the blending weights for current pixel:
			    float4 center = tex2D(_WeightTex, i.uv.xy); 
			    float bottom = tex2D(_WeightTex, offset.zw).g;
			    float right = tex2D(_WeightTex, offset.xy).a;
			    float4 a = float4(center.r, bottom, center.b, right);

			    // Up to 4 lines can be crossing a pixel (one in each edge). So, we perform
			    // a weighted average, where the weight of each line is 'a' cubed, which
			    // favors blending and works well in practice.
			    float4 w = a * a * a;

			    // There is some blending weight with a value greater than 0.0?
			    float sum = dot(w, 1.0);
			    clip (sum - 1e-5);
			        //discard; 

			    float4 finalColor = 0.0;

			    // Add the contributions of the possible 4 lines that can cross this pixel:
			    // Compile bug change float4( 0.0, -a.r, 0.0,  a.g) * (BufferResolution.w) to float4( 0.0, -a.r*BufferResolution.w, 0.0,  a.g*BufferResolution.w) fix compile bug
				//float4 coords = float4( 0.0, -a.r, 0.0,  a.g) * (BufferResolution.w) + i.uv.xyxy;
			    float4 coords = float4(0.0, a.r*_BufferResolution.w, 0.0,  -a.g*_BufferResolution.w)  + i.uv.xyxy;
			    finalColor = tex2D(_BaseTex, coords.xy)*w.r + finalColor;
			    finalColor = tex2D(_BaseTex, coords.zw)*w.g + finalColor;

			    coords = float4(-a.b*_BufferResolution.z,  0.0, a.a*_BufferResolution.z,  0.0)  + i.uv.xyxy;
			    finalColor = tex2D(_BaseTex, coords.xy)*w.b + finalColor;
			    finalColor = tex2D(_BaseTex, coords.zw)*w.a + finalColor;
			    
			    // Normalize the resulting color and we are finished!
			    return finalColor/sum;
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
