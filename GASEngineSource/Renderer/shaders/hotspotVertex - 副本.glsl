attribute vec3 position;
attribute vec2 texCoord0;

varying vec2 vTexCoord0;
varying vec4 vViewPos;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;

uniform float uHotspotIndex;
uniform vec2 frameBufferSize;

// icon size 48 pixel width and 2 pixel padding between them
const vec2 ICON_SIZE = 48.0 / vec2(512.0, -256.0);
const vec2 ICON_AND_PADDING = ICON_SIZE + 2.0 / vec2(512.0, -256.0);

void main(void) 
{
  vec2 offset = vec2(mod(uHotspotIndex, 10.0), floor(uHotspotIndex / 10.0));
  vTexCoord0 = vec2(TexCoord0.x, 1.0 - TexCoord0.y) * ICON_SIZE + offset * ICON_AND_PADDING;

  vViewPos = uModelViewMatrix * vec4(position.xyz, 1.0);
  gl_Position = uProjectionMatrix * vViewPos;
}
