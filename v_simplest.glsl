#version 330 core
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 fragNormal;
out vec3 fragPos;
out vec2 fragTexCoord;

void main() {
    vec4 worldPos = M * vertex;

    // Przekazujemy pozycj� fragmentu w przestrzeni �wiata
    fragPos = vec3(worldPos);

    // Poprawne przekszta�cenie normalnych � M mo�e skalowa�, wi�c robimy transpose(inverse)
    fragNormal = normalize(mat3(transpose(inverse(M))) * normal);

    // Przekazujemy wsp�rz�dne tekstury
    fragTexCoord = texCoord0;

    // Finalna pozycja w klip-space
    gl_Position = P * V * worldPos;
}
