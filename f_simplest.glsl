#version 330 core
in vec3 fragNormal;
in vec3 fragPos;
in vec2 fragTexCoord;
out vec4 FragColor;

uniform sampler2D texUnits[8];
uniform int numTextures;

uniform vec3 viewPos;
uniform vec3 lightPos[5];

// Dodaj nowe uniformy dla efektu upojenia
uniform float time;          // Czas dla animacji
uniform int drinkCounter;    // Liczba wypitych drink�w

void main() {    
    // Pocz�tkowy kolor
    vec4 color = vec4(1.0);
    
    // Modyfikuj wsp�rz�dne tekstury tylko je�li drinkCounter > 0
    vec2 modifiedTexCoord = fragTexCoord;
    
    if (drinkCounter > 0) {
        // Si�a efektu zale�y od liczby drink�w
        float effectStrength = 0.005 * drinkCounter;
        
        // Zniekszta�cenie sinusoidalne
        modifiedTexCoord.x += sin(time * 2.0 + fragTexCoord.y * 10.0) * effectStrength;
        modifiedTexCoord.y += cos(time * 1.7 + fragTexCoord.x * 8.0) * effectStrength;
        
        // Alternatywnie mo�esz doda� efekt "falowania" przestrzeni
        // float wave = sin(time * 3.0 + fragPos.x * 2.0 + fragPos.z * 2.0) * 0.01 * drinkCounter;
        // modifiedTexCoord += vec2(wave);
    }
    
    // Pobierz kolor z tekstur
    for (int i = 0; i < numTextures; ++i) {
       color *= texture(texUnits[i], modifiedTexCoord);
    }
    
    // Dodatkowe efekty wizualne dla wi�kszego upojenia
    if (drinkCounter > 0) {
        // Lekka zmiana kolor�w
        color.r *= 0.9 + 0.1 * sin(time * 3.0);
        color.g *= 0.9 + 0.1 * sin(time * 2.5);
        
        // Rozmycie przy wi�kszej liczbie drink�w
        if (drinkCounter > 2) {
            vec4 blur = vec4(0.0);
            float blurSize = 0.001 * drinkCounter;
            for (int i = 0; i < numTextures; ++i) {
                blur += texture(texUnits[i], modifiedTexCoord + vec2(-blurSize, -blurSize)) / 4.0;
                blur += texture(texUnits[i], modifiedTexCoord + vec2( blurSize, -blurSize)) / 4.0;
                blur += texture(texUnits[i], modifiedTexCoord + vec2(-blurSize,  blurSize)) / 4.0;
                blur += texture(texUnits[i], modifiedTexCoord + vec2( blurSize,  blurSize)) / 4.0;
            }
            color = mix(color, blur, min(drinkCounter * 0.2, 0.7));
        }
    }

    FragColor = color;
}