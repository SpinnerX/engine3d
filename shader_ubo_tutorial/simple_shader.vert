#version 450

//! @note Position and Color used in the Vertex class.
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec3 Normals;
layout(location = 3) in vec2 Uv;


layout(location = 0) out vec3 fragColor;

// we set our descriptor set to binding = 0
// meaning that our descriptor set references to GlobalUbo
// layout(set = 0, binding = 0) uniform GlobalLightUbo {
//     mat4 Model; // light model
//     vec3 Direction; // light dir
// } light_ubo;

layout(binding = 0) uniform GlobalCameraUbo{
    mat4 Projection;
    mat4 View;
    mat4 Model;
    vec3 LightTransform;
    vec4 Color;
} camera_ubo;

layout(push_constant) uniform Push {
    // mat4 Transform; // proj * view
    mat4 Projection;
    mat4 View;
    mat4 Model;
    vec3 LightTransform;
    // vec4 Color;
} push;

// layout(push_constant) uniform Properties {
//     vec4 Color;
// } properties;

// const vec3 dir_to_light = normalize(vec3(1.0, -3.0, -1.0));
// vec3 light_transform = vec3(1.0, -3.0, -1.0);
// const vec3 dir_to_light = normalize(vec3(1.0, -3.0, -1.0));
// vec3 dir_to_light = normalize(push.LightTransform);
// const vec3 dir_to_light = normalize();
vec3 dir_to_light = normalize(push.LightTransform);
const float AMBIENT = 0.2;

void main(){
    // vec3 newPos = vec3(Position.x,Position.y-.5f,Position.z);
    // vec4 worldPositionSpace = push.ModelMatrix * vec3(Position, 1.0);
    // mat4 transform = push.Projection * push.View * push.Mpdel * vec4(Position, 1.0);
    // gl_Position = push.Projection * push.View * push.Model * vec4(Position, 1.0);
    gl_Position = (push.Projection * push.View * push.Model) * vec4(Position, 1.0);
    // gl_Position = (push.Projection * push.View) * vec4(Position, 1.0);

    // gl_Position = transform * vec4(Position, 1.0);

    // mat3 normal_mat = transpose(inverse(mat3(push.ModelMatrix)));
    vec3 normalize_world_space = normalize(mat3(push.Model) * Normals);
    // float LightIntensity = max(dot(normalize_world_space, dir_to_light), 0) / 1.5f;
    float LightIntensity = AMBIENT + max(dot(normalize_world_space, dir_to_light), 0);

    // fragColor = Color;
    fragColor = LightIntensity * Color;
}