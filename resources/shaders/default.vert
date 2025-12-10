#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader
out vec3 w_pos;
out vec3 w_norm;

// Task 6: declare a uniform mat4 to store model matrix
uniform mat4 model;
uniform mat3 inv_t_model;

// Task 7: declare uniform mat4's for the view and projection matrix
uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
}
