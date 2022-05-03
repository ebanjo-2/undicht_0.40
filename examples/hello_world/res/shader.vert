#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec2 aInstancePos;

layout (location = 0) out vec3 fragColor;


layout(binding = 0) uniform UniformBufferObject {
		float time;
		vec2 var;
		vec4 color;
} ubo;




void main() {
		vec3 pos = vec3(aPos.xy + aInstancePos.xy, aPos.z);
		pos.y += ubo.time;
    gl_Position = vec4(pos, 1.0);
    fragColor = ubo.color.xyz;
}
