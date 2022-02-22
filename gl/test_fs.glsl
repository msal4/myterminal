#version 330 core

out vec4 color_frag;
in vec3 color_vert;
in vec2 tex_coord_vert;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main() {
    color_frag = mix(texture(tex1, tex_coord_vert), texture(tex2, vec2(1.0 - tex_coord_vert.x, tex_coord_vert.y)), 0.2);
}