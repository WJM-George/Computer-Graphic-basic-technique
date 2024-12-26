#version 150

in vec4 col;
in vec3 viewPosition;      // view pos of vertex
in vec3 viewNormal;        // view N of vertex

out vec4 c;

// light property:
uniform vec4 La;                      // light ambient component
uniform vec4 Ld;                      // light diffuse component
uniform vec4 Ls;                      // light specular component
uniform vec3 viewLightDirection;      // light direction in view space
uniform vec3 lightPosition;            // pos of light in view space

// material property
uniform vec4 ka;                      // material ambient coefficient
uniform vec4 kd;                      // material diffuse coefficient
uniform vec4 ks;                      // material specular coefficient
uniform float alpha;                  // material shininess coefficient

void main()
{
	//c = col;

	// view direction from the camera to the fragment
	vec3 viewDir = normalize(-viewPosition);

	// reflection direc
	vec3 reflectDir = reflect(-viewLightDirection, viewNormal);

	// dist from light source to fragment
	float q = length(lightPosition - viewPosition);

	// diffuse component
	float diff = max(dot(viewLightDirection, viewNormal), 0.0);
	vec4 diffuse = diff * kd * Ld;

	// specular component
	float spec = pow(max(dot(reflectDir, viewDir), 0.0), alpha);
	vec4 specular = spec * ks * Ls;

	// attenuation
	float attenuation = 1.0 / (0.3 + 0.1 * q + 0.03 * q * q);

	// final color after lighting:
	c = ka * La + diffuse * attenuation + specular * attenuation;
}


