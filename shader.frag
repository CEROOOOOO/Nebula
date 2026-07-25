#version 450
layout(location = 0) in vec3 Colorfromvertex;
layout(location = 1) in vec2 texcodfragfromvertex;
layout(location = 0) out vec4 OutColor;
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    OutColor = vec4(texcodfragfromvertex,0.0,1.0);
    OutColor = texture(texSampler,texcodfragfromvertex);
}
