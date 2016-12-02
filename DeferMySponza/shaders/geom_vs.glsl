#version 330

uniform mat4 proj_view_matrix;
uniform mat4 model_matrix;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec3 varying_position;
out vec3 varying_normal;

void main()
{
	varying_position = vec3(model_matrix * vec4(vertex_position, 1.0));
	varying_normal = vec3(model_matrix * vec4(vertex_normal, 0.0));

	mat4 combined_matrix = proj_view_matrix * model_matrix;
	gl_Position = combined_matrix * vec4(vertex_position, 1.0);
}