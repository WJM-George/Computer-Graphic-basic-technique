#version 150

in vec3 position;
in vec4 color;
in vec3 positionSmooth;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform int renderingMode;
uniform float scale;
uniform float exponent;

//extra cre: Jet color
vec4 JetColorMap(float value) {
	// value should within 0 to 1
	value = clamp(value, 0.0, 1.0);

	// color stops for jetmap
	const int numColors = 8; //# in total
	vec4 colorMap[numColors];
	// black
	colorMap[0] = vec4(0, 0, 0, 1);
	// blue
	colorMap[1] = vec4(0, 0, 0.5, 1);
	// cyan
	colorMap[2] = vec4(0, 0, 1, 1);
	// green
	colorMap[3] = vec4(0, 1, 1, 1);
	// yellow
	colorMap[4] = vec4(1, 1, 0, 1);
	// red
	colorMap[5] = vec4(1, 0, 0, 1);
	// deep Red
	colorMap[6] = vec4(0.5, 0, 0, 1);
	// white
	colorMap[7] = vec4(1, 1, 1, 1);

	//value ?? -> color segment 
	float scaledValue = value * float(numColors - 1);// check index
	int index = int(scaledValue);
	float t = scaledValue - float(index); //get factor

	// linear interpolation of two colors
	if (index < numColors - 1) {
		return mix(colorMap[index], colorMap[index + 1], t);
	}
	return colorMap[numColors - 1]; //lattest color
}

// generate all pt, lin and trian in cpu and bring them to gpu.
void main()
{
	vec3 pos = position;
	if (renderingMode == 0) //mode 1: points, lines, triangles
	{
		col = color; // set color
	}
	else if (renderingMode == 1) //mode 2: smooth
	{
		//When using key "4", color the surface using the JetColorMap function, in the vertex shader.
		// specicially, change the grayscale color x to JetColorMap(x).
		// bigger or smaller, doing all in vertex shader.
		// out-of-bound neighbors -> center vertex

		pos = positionSmooth / 4.0f;

		//scale, exponent -> y	


		float intensity = clamp(2.5 * pow(pos.y, exponent), 0, 1);
		//col = vec4(intensity, intensity, intensity, 1.0);
		col = JetColorMap(intensity);
		pos.y = scale * pow(pos.y, exponent);

	}
	// compute the transformed and projected vertex position (into gl_Position) 
	gl_Position = projectionMatrix * modelViewMatrix * vec4(pos, 1.0f);
}

