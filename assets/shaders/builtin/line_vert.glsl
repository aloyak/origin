#version 410 core
layout (location = 0) in vec3 aCurrentPos;
layout (location = 1) in vec3 aTargetPos;
layout (location = 2) in float aSideSign;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform float u_Thickness;
uniform vec2 u_ScreenSize;

void main() {
    mat4 viewProj = u_Projection * u_View;

    vec4 currentClip = viewProj * vec4(aCurrentPos, 1.0);
    vec4 targetClip  = viewProj * vec4(aTargetPos,  1.0);

    vec2 currentScreen = (currentClip.xy / currentClip.w) * u_ScreenSize;
    vec2 targetScreen  = (targetClip.xy  / currentClip.w) * u_ScreenSize;

    vec2 dir = normalize(targetScreen - currentScreen);
    vec2 normal = vec2(-dir.y, dir.x);

    vec2 offset = normal * aSideSign * (u_Thickness * 0.5);
    currentClip.xy += (offset / u_ScreenSize) * currentClip.w;

    gl_Position = currentClip;
}