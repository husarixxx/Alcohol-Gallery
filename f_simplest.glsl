#version 330 core
in vec3 fragNormal;
in vec3 fragPos;

in vec2 fragTexCoord;
out vec4 FragColor;
uniform sampler2D textureMap0;

uniform vec3 viewPos;
uniform vec3 lightPos[5];

void main() {    
    vec4 texColor = texture(textureMap0, fragTexCoord);
    FragColor = texColor;
}