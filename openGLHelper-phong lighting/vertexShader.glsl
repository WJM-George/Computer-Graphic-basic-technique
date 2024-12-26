#version 150

// for vertices:
in vec3 position;
// this is the normal of the position
in vec4 color; // this is exactly the normal

out vec4 col;
out vec3 viewPosition; // view position of the vertex
out vec3 viewNormal;   // view normal of the vertex

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

// generate all pt, lin and trian in cpu and bring them to gpu.
void main()
{
	col = color;
	vec3 pos = position;
	// for phong: 
	vec4 viewPosition4 = modelViewMatrix * vec4(pos, 1.0f);
	viewPosition = viewPosition4.xyz;
	// color -> view, treat it as N
	viewNormal = normalize((modelViewMatrix * vec4(color.xyz, 0.0f)).xyz); // normal here should just be modelView

	gl_Position = projectionMatrix * viewPosition4;
}

