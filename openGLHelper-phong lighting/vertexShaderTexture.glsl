#version 150

// for vertices:
in vec3 position;
// for UV coor
in vec2 texCoord;

// texture coordinate to fragment shader:
out vec2 tc;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

// generate all pt, lin and trian in cpu and bring them to gpu.
void main()
{
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f); 
  tc = texCoord; 
}

