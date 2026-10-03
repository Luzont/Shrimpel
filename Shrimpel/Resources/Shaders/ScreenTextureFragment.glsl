#version 460 core

out vec4 FragColor;

in vec2 TextureCoordinate;

uniform sampler2D ScreenTexture;

void main() 
{
	FragColor = texture(ScreenTexture, TextureCoordinate);
}

/* Postproccesing effects
1. EVIL OPENGL:
	FragColor = vec4(vec3(1.0 - texture(ScreenTexture, TextureCoordinate)), 1.0);

2. 1950s OpenGL:
	FragColor = texture(ScreenTexture, TextureCoordinate);
	float ColorAverage = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
	FragColor = vec4(ColorAverage, ColorAverage, ColorAverage, 1.0);

3. ULTRA SATURATED OPENGL
	You also need this before the main():
	const float Offset = 1.0/300.0;


	vec2 Offsets[9] = vec2[](
		vec2(-Offset, Offset), // top-left
		vec2(0.0f, Offset),    // top-center
		vec2(Offset, Offset),  // top-right
		vec2(-Offset, 0.0f),   // center-left
		vec2(0.0f, 0.0f),      // center-center
		vec2(Offset, 0.0f),    // center-right
		vec2(-Offset, -Offset),// bottom-left
		vec2(0.0f, Offset),    // bottom-center
		vec2(Offset, -Offset)  // bottom-right
	);

	float Kernel[9] = float[](
		-1, -1, -1,
		-1, 9, -1,
		-1, -1, -1
	);

	vec3 SampleTexture[9];
	
	for (int i = 0; i < 9; i++)
		SampleTexture[i] = vec3(texture(ScreenTexture, TextureCoordinate.st + Offsets[i]));

	vec3 Color = vec3(0.0);

	for (int i = 0; i < 9; i++)
		Color += SampleTexture[i] * Kernel[i];

	FragColor = vec4(Color, 1.0);

4. Blured OpenGL
	just deepfried but change the kernel to this:
	float Kernel[9] = float[](
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16
	);

5. D E E P F R I E D   O P E N G L:
	just deepfried but change the kernel to this:
		float Kernel[9] = float[](
			1, 1, 1,
			1, -8, 1,
			1, 1, 1
		);

E X T R A S ! ! !
1. Pixelated:
	You also need this before the main():
	vec2 Resolution = vec2(800, 600);
	const float PixelSize = 8.0;

	
	vec2 PixelatedUV = floor(TextureCoordinate * Resolution / PixelSize) * PixelSize / Resolution;
	FragColor = texture(ScreenTexture, PixelatedUV);

2. hella bright:
	this is the kernel:
	float Kernel[9] = float[](
		5, 4, 3,
		7, 47, 6,
		9, 8, 5
	);
*/