#version 330 core

in vec2 texCoord0;
out vec4 fragColor;

uniform sampler2D texUnits[4];
uniform int numTextures;
uniform float time; // Czas do animacji efektu
uniform int drunkLevel; // Poziom upojenia (0 = brak efektu)

void main() {
    vec2 distortedCoords = texCoord0;

    if (drunkLevel > 0) {
        // Efekt falowania (im wiêkszy drunkLevel, tym silniejsze zaburzenie)
        float waveIntensity = 0.02 * drunkLevel;
        float waveSpeed = 2.0;
        
        distortedCoords.x += sin(distortedCoords.y * 10.0 + time * waveSpeed) * waveIntensity;
        distortedCoords.y += cos(distortedCoords.x * 8.0 + time * waveSpeed) * waveIntensity;
    }

    fragColor = texture(texUnits[0], distortedCoords);
}