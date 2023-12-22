uniform sampler2D maskTexture;
uniform vec2 texSize;
varying vec2 texcoord0;

void main() {
    vec2 invSize = 1.0 / texSize;
    vec4 uvOffset = vec4(1.0, 0.0, 0.0, 1.0) * vec4(invSize, invSize);
    vec4 c1 = texture2D( maskTexture, texcoord0 + uvOffset.xy);
    vec4 c2 = texture2D( maskTexture, texcoord0 - uvOffset.xy);
    vec4 c3 = texture2D( maskTexture, texcoord0 + uvOffset.yw);
    vec4 c4 = texture2D( maskTexture, texcoord0 - uvOffset.yw);
    float diff1 = (c1.r - c2.r)*0.5;
    float diff2 = (c3.r - c4.r)*0.5;
    float d = length( vec2(diff1, diff2) );
    gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0) * vec4(d);
}