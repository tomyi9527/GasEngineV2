attribute vec4 position;
attribute vec4 color;
attribute vec2 uv;

varying vec2 texCoord0;
varying vec4 vertexViewPos;
varying vec4 vertexColor;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec2 frameBufferSize;
uniform vec2 hotspotSize;

// icon size 48 pixel width and 2 pixel padding between them
const vec2 ICON_SIZE = 48.0 / vec2(512.0, 256.0);
const vec2 ICON_AND_PADDING = ICON_SIZE + 2.0 / vec2(512.0, 256.0);

void main(void) 
{
	vertexColor = color;

	float hotspotIndex = position.w - 1.0;

	vec2 uv01 = (uv.xy + 1.0) / 2.0;
	uv01.y = 1.0 - uv01.y;

	vec2 offset = vec2(mod(hotspotIndex, 10.0), floor(hotspotIndex / 10.0)); // col , row
	texCoord0 = uv01 * ICON_SIZE + offset * ICON_AND_PADDING;

	vertexViewPos = viewMatrix * vec4(position.xyz, 1.0);
	vec4 projectedPos = projectionMatrix * vertexViewPos;
	vec2 screenSpace = projectedPos.xy / projectedPos.w;
	screenSpace += (uv.xy * (1.0 / frameBufferSize) * hotspotSize);

	gl_Position = vec4(screenSpace, 1.0, 1.0);
}
