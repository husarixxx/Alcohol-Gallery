#version 330 core
in vec3 fragNormal;
in vec3 fragPos;

in vec2 fragTexCoord;
out vec4 FragColor;

uniform sampler2D texUnits[8];
uniform int numTextures;

uniform vec3 viewPos;
uniform vec3 lightPos[5];

void main() {    
    vec4 color = vec4(1.0);
    for (int i = 0; i < numTextures; ++i){
       color *= texture(texUnits[i], fragTexCoord);
    }
    FragColor = color;
}