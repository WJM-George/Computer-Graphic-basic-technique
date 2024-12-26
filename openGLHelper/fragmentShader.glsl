#version 150

in vec4 col;
out vec4 c;

void main()
{
	// compute the final pixel color
  // color is calculated in fragment shader, related element: scale exponent.
	c = col;
}

