#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;

layout(binding = 0) uniform UniformBufferObject {

	mat4 proj;
	mat4 view;

} ubo;

layout (location = 0) out vec2 uv;

void main() {

    uv = aUV;
    gl_Position = ubo.proj * ubo.view * vec4(aPos, 1.0);

}
