#version 330 core

in vec4 fragWorldPosition; // From vertex shader
out vec4 color;

uniform float scale;
uniform vec3 offset;
uniform vec3 color1;
uniform vec3 color2;

void main() {
   bool x = int((fragWorldPosition.x + offset.x+200) * scale) % 2 != 0;
    bool y = int((fragWorldPosition.y + offset.y) * scale) % 2 != 0;
    bool z = int((fragWorldPosition.z + offset.z) * scale) % 2 != 0;

    // Perform XOR operation on the boolean values
    bool xorXY = x != y;
    
    if (xorXY != z) {
        color = vec4(0, 0, 255.0f, 1.0f);
    } else {
        color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}
