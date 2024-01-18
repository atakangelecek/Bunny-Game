#version 330 core

in vec3 FragPosition; 
in vec3 Normal; 

 
uniform vec3 eyePos; 
uniform vec3 lightColor; 
uniform vec3 objectColor; 
uniform vec3 lightPosition;

out vec4 FragColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse 
    vec3 normalized = normalize(Normal);
    vec3 lightDirection = normalize(lightPosition - FragPosition);
    float diff = max(dot(normalized, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDirection = normalize(eyePos - FragPosition);
    vec3 reflectDirection = reflect(-lightDirection, normalized);
    
    float specularStrength = 0.5;
      
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
