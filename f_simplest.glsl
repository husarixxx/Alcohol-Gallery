#version 330 core
in vec3 fragNormal;
in vec3 fragPos;
in vec2 fragTexCoord;
out vec4 FragColor;

uniform sampler2D texUnits[8];
uniform int numTextures;

uniform vec3 viewPos;
uniform vec3 lightPos[6];
uniform vec4 lightColor; 
uniform vec3 lightDir;       
uniform float lightCutoff;   
uniform float lightOuterCutoff;
uniform float time;
uniform int drinkCounter;    

void main() {    
    vec4 color = vec4(1.0);
    
    vec2 modifiedTexCoord = fragTexCoord;
    
    if (drinkCounter > 0) {
        float effectStrength = 0.005 * drinkCounter;        
        modifiedTexCoord.x += sin(time * 2.0 + fragTexCoord.y * 10.0) * effectStrength;
        modifiedTexCoord.y += cos(time * 1.7 + fragTexCoord.x * 8.0) * effectStrength;       
    }
    
    for (int i = 0; i < numTextures; ++i) {
       color *= texture(texUnits[i], modifiedTexCoord);
    }
    
    if (drinkCounter > 0) {
        color.r *= 0.9 + 0.1 * sin(time * 3.0);
        color.g *= 0.9 + 0.1 * sin(time * 2.5);
        
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
    
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = vec3(0.0);

    vec3 eyePos = lightPos[0];
    vec3 eyeDir = normalize(-lightDir);
    float eyeCutoff = cos(radians(30.0)); 

    for (int i = 0; i < 6; ++i) {
        vec3 lightToFrag = fragPos - lightPos[i];   
        float distance = length(lightToFrag);
        vec3 lightDirNorm = normalize(-lightToFrag);
        vec3 currentLightDir;
        float cutoff = 0.0;
        float outerCutoff = 0.0;
        float lightStrength = 0.0;
        const float camLightConstant = 1.0;
        const float camLightLinear = 0.09;
        const float camLightQuadratic = 0.032;

        if (i == 0) {
            currentLightDir = normalize(-lightDir); 
            cutoff = cos(radians(50.0));
            outerCutoff = cos(radians(55.0));
            lightStrength = 0.7;
        } 
        else if (i == 5){
            currentLightDir = vec3(0.0, 1.0, 0.0);
            cutoff = cos(radians(40.0));
            outerCutoff = cos(radians(50.0));
            lightStrength = 0.5;
        }
        else {
            currentLightDir = vec3(0.0, 1.0, 0.0);
            cutoff = lightCutoff; 
            outerCutoff = lightOuterCutoff;
            lightStrength = 1.0;
        }

        float theta = dot(lightDirNorm, normalize(currentLightDir)); 
        float intensity = smoothstep(outerCutoff, cutoff, theta) * lightStrength;
        float diff = max(dot(normal, lightDirNorm), 0.0);
        
        vec3 reflectDir = reflect(-lightDirNorm, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

        vec3 ambient = 0.3 * lightColor.rgb * color.rgb;
        
        float shadowFactor = 1.0;

        vec3 diffuse = diff * lightColor.rgb * color.rgb;
        vec3 specular = spec * lightColor.rgb * color.rgb;
        result += lightStrength * (ambient + shadowFactor * (diffuse + specular)) * intensity;

    }
    result = clamp(result, 0.0, 1.0);   
    FragColor = vec4(result, color.a);
}
