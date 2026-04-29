#version 330 core
in vec2 TexCoords;
in vec3 NormalDir;       
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseTexture;
uniform vec3 lightPos;

void main() {
    vec4 lightColor = vec4(1.0);
    float ambientStrength = 0.1;
    vec4 ambient = lightColor * ambientStrength;

    vec3 norm = normalize(NormalDir);          
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); 
    vec4 diffuse = diff * lightColor;          

    FragColor = (ambient + diffuse) * texture(diffuseTexture, TexCoords);
}
