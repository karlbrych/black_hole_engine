#version 330 core
in vec2 TexCoords;
in vec3 NormalDir;
in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D diffuseTexture;
uniform vec3 lightPos;
uniform vec3 cameraPos;

void main() {
    vec4 lightColor = vec4(1.0);
    float ambientStrength = 0.1;
    vec4 ambient = lightColor * ambientStrength;

    vec3 norm = normalize(NormalDir);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);     
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specular = specularStrength * spec * lightColor; 

    FragColor = (ambient + diffuse + specular) * texture(diffuseTexture, TexCoords);
}