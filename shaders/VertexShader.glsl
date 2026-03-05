#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniforms
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 object_color;

uniform int is_broken; // 1 = DA, 0 = NU
uniform float time;

// Output
out vec3 frag_normal;
out vec3 frag_color;

void main()
{
    frag_normal = v_normal;
    frag_color = object_color;

    vec3 new_pos = v_position;

    if (is_broken == 1) {
        float displacementY = sin(new_pos.x * 4.0) * 0.35; 
        float displacementZ = cos(new_pos.x * 5.0) * 0.2;

        new_pos.y += displacementY;
        new_pos.z += displacementZ;
    }

    gl_Position = Projection * View * Model * vec4(new_pos, 1.0);
}