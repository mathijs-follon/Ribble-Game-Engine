#version 450
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(push_constant) uniform FragPushConstants {
    vec4 color;
} pc;
void main() {
    outColor = pc.color;
}
