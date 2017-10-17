#version 430

layout(location=0) in vec4 position;
layout(location=1) in vec4 normal;
layout(location=2) in vec4 texCoord;

out VS_OUT {
	vec4 normal;
	vec4 texCoord;
	vec3 L;
	vec3 V;
	vec3 vertWorld;
} vs_out;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(void) {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;
	vs_out.normal = normal;
	vs_out.texCoord = texCoord;
	vs_out.L = vec3(10.f) - position.xyz;
	vs_out.V = -position.xyz;
	vs_out.vertWorld = vec3(modelMatrix * vec4(position.xyz, 1.0));
}