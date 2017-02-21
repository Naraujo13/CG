// ================
// Geometry shader:
// ================
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
layout(location = 1) in vec2 explosionUV[];
in vec3 LightDirection_cameraspace[];
out vec2 UV; 
out vec3 vertexpos;
out vec3 ExpLightDirection_cameraspace;

uniform float time;
uniform vec3 LightPosition_worldspace;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 10.0f;
    vec3 direction = normal * ((sin(time) + 1.0f) / 2.0f) * magnitude;
    return position + vec4(direction, 0.0f);
   return position;
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main() {    

    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    UV = explosionUV[0];
	ExpLightDirection_cameraspace = LightDirection_cameraspace[0];
    EmitVertex();

    gl_Position = explode(gl_in[1].gl_Position, normal);
    UV = explosionUV[1];
	ExpLightDirection_cameraspace = LightDirection_cameraspace[1];
    EmitVertex();

    gl_Position = explode(gl_in[2].gl_Position, normal);
    UV = explosionUV[2];
	ExpLightDirection_cameraspace = LightDirection_cameraspace[2];
    EmitVertex();
    EndPrimitive();

}
