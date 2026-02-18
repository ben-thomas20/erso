#version 410 core
// Sample and output a single texture (used to blit an FBO to the swapchain).

in  vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    FragColor = texture(u_Texture, vUV);
}
