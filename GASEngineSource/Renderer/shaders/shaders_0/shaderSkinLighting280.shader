Shader "Custom/shaderSkinLighting280" 
{
	Properties 
	{
		_DepthSampler ("Base (RGB)", 2D) = "white" {}
		_SrcSampler ("Base (RGB)", 2D) = "white" {}
		_DiffuseG ("Base (RGB)", 2D) = "white" {}
	}

	SubShader 
	{
		Pass
		{
		
			Stencil
			{
                Ref			2
                ReadMask	2
                WriteMask	2
                Comp 		Equal
                Pass 		Keep
                Fail		Keep
                ZFail 		Keep
            }
            
            ZTest Off
          	ZWrite Off
		
		CGPROGRAM
		#pragma target 3.0
		#pragma vertex vert_img
		#pragma fragment frag
		#include "UnityCG.cginc"

		sampler2D _DepthSampler;
		sampler2D _SrcSampler;
		sampler2D _DiffuseG;
		
//		#define SSSS_N_SAMPLES 25
//		static float4 kernel[SSSS_N_SAMPLES] = 
//		{
//		    float4(0.530605, 0.613514, 0.739601, 0),
//		    float4(0.000973794, 1.11862e-005, 9.43437e-007, -3),
//		    float4(0.00333804, 7.85443e-005, 1.2945e-005, -2.52083),
//		    float4(0.00500364, 0.00020094, 5.28848e-005, -2.08333),
//		    float4(0.00700976, 0.00049366, 0.000151938, -1.6875),
//		    float4(0.0094389, 0.00139119, 0.000416598, -1.33333),
//		    float4(0.0128496, 0.00356329, 0.00132016, -1.02083),
//		    float4(0.017924, 0.00711691, 0.00347194, -0.75),
//		    float4(0.0263642, 0.0119715, 0.00684598, -0.520833),
//		    float4(0.0410172, 0.0199899, 0.0118481, -0.333333),
//		    float4(0.0493588, 0.0367726, 0.0219485, -0.1875),
//		    float4(0.0402784, 0.0657244, 0.04631, -0.0833333),
//		    float4(0.0211412, 0.0459286, 0.0378196, -0.0208333),
//		    float4(0.0211412, 0.0459286, 0.0378196, 0.0208333),
//		    float4(0.0402784, 0.0657244, 0.04631, 0.0833333),
//		    float4(0.0493588, 0.0367726, 0.0219485, 0.1875),
//		    float4(0.0410172, 0.0199899, 0.0118481, 0.333333),
//		    float4(0.0263642, 0.0119715, 0.00684598, 0.520833),
//		    float4(0.017924, 0.00711691, 0.00347194, 0.75),
//		    float4(0.0128496, 0.00356329, 0.00132016, 1.02083),
//		    float4(0.0094389, 0.00139119, 0.000416598, 1.33333),
//		    float4(0.00700976, 0.00049366, 0.000151938, 1.6875),
//		    float4(0.00500364, 0.00020094, 5.28848e-005, 2.08333),
//		    float4(0.00333804, 7.85443e-005, 1.2945e-005, 2.52083),
//		    float4(0.000973794, 1.11862e-005, 9.43437e-007, 3),
//		};
		
		#define SSSS_N_SAMPLES 11
		static float4 kernel[SSSS_N_SAMPLES] = 
		{
		    float4(0.560479, 0.669086, 0.784728, 0),
		    float4(0.00471691, 0.000184771, 5.07566e-005, -2),
		    float4(0.0192831, 0.00282018, 0.00084214, -1.28),
		    float4(0.03639, 0.0130999, 0.00643685, -0.72),
		    float4(0.0821904, 0.0358608, 0.0209261, -0.32),
		    float4(0.0771802, 0.113491, 0.0793803, -0.08),
		    float4(0.0771802, 0.113491, 0.0793803, 0.08),
		    float4(0.0821904, 0.0358608, 0.0209261, 0.32),
		    float4(0.03639, 0.0130999, 0.00643685, 0.72),
		    float4(0.0192831, 0.00282018, 0.00084214, 1.28),
		    float4(0.00471691, 0.000184771, 5.07565e-005, 2),
		};
		
		float4 _DepthParam; //n*f, f, f-n, n/a Vector4(2250000, 150000, 149985, 0)
		//scatterFactor/backBufSize.x, scatterFactor/backBufSize.y, 1.0f/backBufSize.x, 1.0f/backBufSize.y
		//scatterFactor = config.mSubSurfaceScatterFactor*0.0025f; mSubSurfaceScatterFactor = 10
		//scatterFactor = 0.025
		float4 _BufferResolution;
		
		float GetLinearZ(float2 uv)
		{
		    float depthM = tex2D(_DepthSampler, uv).r;
		    return depthM;
		}
		
		float4 ScatterBlur(float2 uv, float2 step, float stepLen)
		{
		    float4 colorM = tex2D(_SrcSampler, uv);
		    float depthM = GetLinearZ(uv);    

		    // Accumulate center sample, multiplying it with its gaussian weight:
		    float3 colorBlurred = kernel[0].xyz*colorM.xyz;    

		    float2 finalStep = step/depthM;

		    for(int i = 1; i < SSSS_N_SAMPLES; i++)
		    {
		        // Fetch color and depth for current sample:
		        float2 offset = uv + kernel[i].a * finalStep;
		        float4 color = tex2D(_SrcSampler, offset);       

				//skin scattering is the first pass of lighting
				//only title rendering is ahead of it, so we can use color(rgb is black means it's not skin, alpha!=1.0f, means it's title) to avoid unnecessary scattering
				if(any(color.rgb)&&color.a==1.0f)
				{
					float depth = GetLinearZ(offset);

					// If the difference in depth is huge, we lerp color back to "colorM":
					float s = min(50000.0f * stepLen * abs(depthM - depth), 1.0);
					color.rgb = lerp(color.rgb, colorM.rgb, s);

					// Accumulate:
					colorBlurred += kernel[i].xyz * color.rgb;
				}
				else
				{			
					colorBlurred += kernel[i].xyz * colorM.xyz;
				}        
		    }

		    return float4(colorBlurred, 1.0f);
		}

		#define SSS_SCALE_FACTOR		4.0f
		
		float4 frag(v2f_img i) : COLOR
		{
			float2 scatterFactor = _BufferResolution.xy;
			float stepLen = scatterFactor.y;
    		float2 step = float2(0.0f, scatterFactor.y);
    		float2 uv = i.uv + 0.5f*_BufferResolution.zw;

    		float4 result = ScatterBlur(i.uv, step, stepLen);
	
			float4 diffuse = tex2D(_DiffuseG, i.uv);	
			diffuse.xyz = pow(abs(diffuse.xyz), 2.2f);	
			result.xyz = result.xyz*diffuse.xyz*SSS_SCALE_FACTOR;

			return result;
		}

		ENDCG
		} 
	}
}