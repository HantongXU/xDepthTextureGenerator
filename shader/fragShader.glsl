#version 430 core

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 LocalPos0;

uniform sampler2D sampler;   																				

layout (location = 0) out vec3 FragColor;
layout (location = 1) out uint FragDepth;
layout (location = 2) out vec3 FragNormal;

void main(void) 
{
	FragColor = (texture2D(sampler, TexCoord0.xy)).zyx;
	
	FragDepth = uint(LocalPos0.z * 1000.0);
	
	FragNormal = normalize(Normal0);
}

