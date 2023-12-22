Shader "Custom/shaderMLAAFindLShape325" 
{
	Properties 
	{
		_DiscontinuityTex ("Base (RGB)", 2D) = "white" {}
		_AreaCalcTex ("Base (RGB)", 2D) = "white" {}		
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
			#pragma target 3.0
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			uniform sampler2D _DiscontinuityTex;
			uniform sampler2D _AreaCalcTex;
			uniform float4 _BufferResolution;
			
			#define MAX_SEARCH_STEPS 8    
			#define MAX_DISTANCE 32
			//采中间，利用硬件插值可以同时检测相邻两个像素，如果相邻两个像素都是1，结果是1，如果相邻一个是1一个是0，则是0.5，最后两个0则是0
			#define SEARCH_START_BIAS 1.5f 
			#define SEARCH_STRIDE 2.0f
			
			float4 tex2Dlevel0(sampler2D map, float2 texcoord)
			{
			    return tex2Dlod(map, float4(texcoord, 0.0, 0.0));
			}

			float SearchXLeft(float2 UV)
			{			    
			    float i;
			    float e = 0.0f;
			    for (i = -SEARCH_START_BIAS; i > -SEARCH_STRIDE*MAX_SEARCH_STEPS; i -= SEARCH_STRIDE)
			    {
			    	float4 biasedUV = float4(UV + _BufferResolution.zw*float2(i, 0.0f), 0, 0);
			        e = tex2Dlod(_DiscontinuityTex, biasedUV).y;
			        
			        //[flatten]
			        if (e < 0.9f) // We compare with 0.9 to prevent bilinear access precision problems.
			        {
			        	break;
			        }
			    }
			    //如果break,e等于0.5f或0
			    return max(i + SEARCH_START_BIAS - SEARCH_STRIDE*e, -SEARCH_STRIDE*MAX_SEARCH_STEPS);
			}

			float SearchXRight(float2 UV)
			{
			    float i;
			    float e = 0.0f;
			    for (i = SEARCH_START_BIAS; i < SEARCH_STRIDE*MAX_SEARCH_STEPS; i += SEARCH_STRIDE)
			    {
			        float4 biasedUV = float4(UV + _BufferResolution.zw*float2(i, 0.0f), 0, 0);
			        e = tex2Dlod(_DiscontinuityTex, biasedUV).y;
			        
			        //[flatten]
			        if (e < 0.9f)
			        {
			        	break;
			        }
			    }
			    
			    return min(i - SEARCH_START_BIAS + SEARCH_STRIDE*e, SEARCH_STRIDE*MAX_SEARCH_STEPS);
			}

			float SearchYUp(float2 UV)
			{
			    float i;
			    float e = 0.0;
			    for (i = SEARCH_START_BIAS; i < SEARCH_STRIDE*MAX_SEARCH_STEPS; i += SEARCH_STRIDE)			    
			    {
			        float4 biasedUV = float4(UV + _BufferResolution.zw*float2(0.0f, i), 0, 0);
			        e = tex2Dlod(_DiscontinuityTex, biasedUV).x;
			        
			        //[flatten]
			        if (e < 0.9)
			        {
			        	break;
			        }
			    }
			    
			    return min(i - SEARCH_START_BIAS + SEARCH_STRIDE*e, SEARCH_STRIDE*MAX_SEARCH_STEPS);
			}

			float SearchYDown(float2 UV)
			{
			    float i;
			    float e = 0.0;
			    for (i = -SEARCH_START_BIAS; i > -SEARCH_STRIDE*MAX_SEARCH_STEPS; i -= SEARCH_STRIDE)
			    {
			        float4 biasedUV = float4(UV + _BufferResolution.zw*float2(0.0f, i), 0, 0);
			        e = tex2Dlod(_DiscontinuityTex, biasedUV).x;
			        
			        //[flatten]
			        if (e < 0.9)
			        {
			        	break;
			        }
			    }
			    
			    return max(i + SEARCH_START_BIAS - SEARCH_STRIDE*e, -SEARCH_STRIDE*MAX_SEARCH_STEPS);			    
			}

			float2 Area(float2 distance, float e1, float e2)
			{
			     // * By dividing by areaSize - 1.0 below we are implicitely offsetting to
			     //   always fall inside of a pixel
			     // * Rounding prevents bilinear access precision problems
			    float areaSize = MAX_DISTANCE * 5.0;
			    float2 pixcoord = MAX_DISTANCE * round(4.0 * float2(e1, e2)) + distance;
			    float2 texcoord = pixcoord / (areaSize - 1.0);
			    //for OpenGL
			    texcoord.y = 1.0 - texcoord.y;
			    return tex2Dlevel0(_AreaCalcTex, texcoord).ra;
			}

			float4 frag(v2f_img i) : COLOR
			{		
				float4 areas = 0.0;
			    float2 e = tex2D(_DiscontinuityTex, i.uv).xy;

			    //[branch]
			    if(e.y) //水平的不连续性
			    { 
			    	// Edge at north
			        // Search distances to the left and to the right:
			        float2 d = float2(SearchXLeft(i.uv), SearchXRight(i.uv));
			        // Now fetch the crossing edges. Instead of sampling between edgels, we
			        // sample at -0.25, to be able to discern what value has each edgel:
			        // d.y + 1.0 是因为左边的缘故，所以在右边应该是下一个像素
			        // 采样的时候向纹理上方偏移0.25个像素，确定垂直方向不连续的形状可能有三种
			        //|_ 0.25  
			        //
			        
			        // _
			        //|  0.75
			        
			        //|_ 1.0
			        //|
			        float4 coords = float4(d.x, 0.25, d.y + 1.0, 0.25)*_BufferResolution.zwzw + i.uv.xyxy;

					float e1 = tex2Dlevel0(_DiscontinuityTex, coords.xy).r;
					float e2 = tex2Dlevel0(_DiscontinuityTex, coords.zw).r;
			        		        
			        // Ok, we know how this pattern looks like, now it is time for getting
			        // the actual area:
			        areas.rg = Area(abs(d), e1, e2);//上下权重
			    }

			    //[branch]
			    if(e.x) //上下的不连续性
			    { 
			    	// Edge at west
			        // Search distances to the top and to the bottom:
			        float2 d = float2(SearchYUp(i.uv), SearchYDown(i.uv));
			        //float2 k = abs(d);
					//areas.rg = float2(k.x/(k.x+k.y), k.y/(k.x+k.y));
					
			        // Now fetch the crossing edges (yet again):
			        float4 coords = float4(-0.25, d.x, -0.25, d.y - 1.0)*_BufferResolution.zwzw + i.uv.xyxy;
			        
					float e1 = tex2Dlevel0(_DiscontinuityTex, coords.xy).g;
					float e2 = tex2Dlevel0(_DiscontinuityTex, coords.zw).g;

			        // Get the area for this direction:
			        areas.ba = Area(abs(d), e1, e2); //左右权重
    			}

    			return areas;
			}

			ENDCG
		}
		//< Pass 
	}
	//< SubShader
}
