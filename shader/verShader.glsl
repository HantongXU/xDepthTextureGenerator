#version 430 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;   

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;

void main(void)
{
    gl_Position = projectionMatrix * modelViewMatrix * vec4(Position, 1.0);
	 
	TexCoord0   = TexCoord;
	//TexCoord0.x=1.0*786/4096;
	//TexCoord0.y=1.0*570/4096;
    Normal0     = Normal;//(modelViewMatrix * vec4(Normal, 0.0)).xyz;
    LocalPos0   = - (modelViewMatrix * vec4(Position, 1.0)).xyz;  
}

