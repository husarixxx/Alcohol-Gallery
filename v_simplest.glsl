#version 330 core
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 viewPos;

out vec3 fragNormal;
out vec3 fragPos;

out vec2 fragTexCoord;

void main() {
    vec4 worldPos = M * vertex;
    
    gl_Position = P * V * worldPos;
    fragTexCoord = texCoord0;
}