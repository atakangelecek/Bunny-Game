#version 330 core

layout (location = 0) in vec3 aPosition; 
layout (location = 1) in vec3 aNormal;   

out vec3 Normal;       
out vec3 FragPosition;      

uniform mat4 modelingMatrix; 
uniform mat4 viewingMatrix;  
uniform mat4 projectionMatrix; 
uniform vec3 offset; 
uniform float scale; 

void main()
{
    vec4 vertexPosition = modelingMatrix * vec4(aPosition, 1.0);
    vertexPosition = vec4(vertexPosition.x * scale + offset.x, vertexPosition.y * scale + offset.y, vertexPosition.z * scale + offset.z, 1.0);
    FragPosition = vec3(vertexPosition); 
    
    // Project the position to image plane
    gl_Position = projectionMatrix * viewingMatrix * vertexPosition;
    Normal = mat3(transpose(inverse(modelingMatrix))) * aNormal;
}
