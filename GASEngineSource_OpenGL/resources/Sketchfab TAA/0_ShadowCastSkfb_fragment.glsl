#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
 #else
 precision mediump float;
#endif
#define SHADER_NAME ShadowCastSkfb
#define _PCFx1


uniform vec4 uShadowDepthRange;

varying vec4 vViewVertex;




float decodeFloatRGBA( vec4 rgba ) {
    return dot( rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0) );
}

vec4 encodeFloatRGBA( float v ) {
    vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
    enc = fract(enc);
    enc -= enc.yzww * vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
    return enc;
}

vec2 decodeHalfFloatRGBA( vec4 rgba ) {
    return vec2(rgba.x + (rgba.y / 255.0), rgba.z + (rgba.w / 255.0));
}

vec4 encodeHalfFloatRGBA( vec2 v ) {
    const vec2 bias = vec2(1.0 / 255.0, 0.0);
    vec4 enc;
    enc.xy = vec2(v.x, fract(v.x * 255.0));
    enc.xy = enc.xy - (enc.yy * bias);

    enc.zw = vec2(v.y, fract(v.y * 255.0));
    enc.zw = enc.zw - (enc.ww * bias);
    return enc;
}



vec4 computeShadowDepth(const in vec4 fragEye,
                        const in vec4 shadowRange){
    // distance to camera
    float depth =  -fragEye.z * fragEye.w;
    // most precision near 0, make sure we are near 0 and in  [0,1]
    depth = (depth - shadowRange.x ) * shadowRange.w;

    vec4 outputFrag;

#if defined (_FLOATTEX) 
    outputFrag = vec4(depth, 0.0, 0.0, 1.0);
#else
    outputFrag = encodeFloatRGBA(depth);
#endif

    return outputFrag;
}

void main() {
gl_FragColor = computeShadowDepth( vViewVertex, uShadowDepthRange );

}