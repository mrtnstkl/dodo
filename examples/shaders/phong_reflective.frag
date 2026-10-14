#version 430 core


// =====( INPUT )===== //

in vertex_data{
	vec3 world_position;
	vec3 world_normal;
	vec2 uv;
}vert;

// =====( UNIFORMS )===== //

uniform vec3 model_color;

uniform vec4 material_coefficients;
#define ambient_constant material_coefficients.x
#define diffuse_constant material_coefficients.y
#define specular_constant material_coefficients.z
#define specular_alpha material_coefficients.a

struct DirectionalLight
{
	vec3 color;
	vec3 direction;
};
struct PointLight
{
	vec3 color;
	vec3 position;
	vec3 attenuation;
};

uniform uint directional_light_array_size;
uniform DirectionalLight directional_light[8];

uniform uint point_light_array_size;
uniform PointLight point_light[8];

uniform vec3 camera_position;

uniform sampler2D color_texture;
uniform sampler2D specular_map;
uniform samplerCube environment_map;

// =====( OUTPUT )===== //

out vec4 fragColor;


// =====( FUNCTIONS )===== //

vec3 phong_lighting(
	const vec3 position_,
	const vec3 normal_,
	const vec3 model_color_,
	const vec3 light_color_,
	const vec3 light_direction_,
	const vec3 camera_position_,
	const float diffuse_constant_,
	const float specular_constant_,
	const float specular_alpha_,
	const float light_distance_,
	const vec3 attenuation_
){
	// -----( diffuse )----- //
	const vec3 L_diffuse=
		light_color_* model_color_ *
		diffuse_constant_*
		clamp(dot(normal_,light_direction_),0,1)
	;

	// -----( specular )----- //
	const vec3 L_specular=
		light_color_*
		specular_constant_*
		pow(
			max(
				dot(
					// vector from fragment to camera
					normalize(camera_position_-position_),
					// vector of the reflected light
					-reflect(light_direction_,normal_)
				),
				0.0
			),
			specular_alpha_
	)
	;
	
	// attenuation
	float attenuation=1.0;
	if(light_distance_ >= 0){
		attenuation=
			1.0/
			(attenuation_.x+attenuation_.y*light_distance_+
			attenuation_.z*light_distance_*light_distance_);
	}
	
	return (L_diffuse+L_specular)*attenuation;
}


// =====( MAIN )===== //

void main(){
	
	const vec3 world_normal = normalize(vert.world_normal);
	const vec3 color_from_texture = texture(color_texture, vert.uv).rgb;
	const float specular_from_map = texture(specular_map, vert.uv).r;

	// ambient light
	fragColor = vec4( color_from_texture * ambient_constant ,1);

	// add contribution of directional lights
	for(uint i=0;i<directional_light_array_size;i++){
		fragColor.rgb +=phong_lighting(
			vert.world_position,
			world_normal,
			color_from_texture,
			directional_light[i].color,
			directional_light[i].direction,
			camera_position,
			diffuse_constant,
			specular_from_map,
			specular_alpha, 
			-1, vec3(0)
		);
	}
	
	// add contribution of point lights
	for(uint i=0;i<point_light_array_size;i++){
		fragColor.rgb+=phong_lighting(
			vert.world_position,
			world_normal,
			color_from_texture,
			point_light[i].color,
			normalize(point_light[i].position - vert.world_position),
			camera_position,
			diffuse_constant,
			specular_from_map,
			specular_alpha,
			length(point_light[i].position - vert.world_position),
			point_light[i].attenuation
		);
	}

	// add environment reflection
	fragColor += specular_from_map * texture(environment_map, reflect((vert.world_position - camera_position), world_normal ));
}