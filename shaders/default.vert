#version 330 core

//Position/Coords
layout (location = 0) in vec3 aPos;

//Normal coords (not necessarily normalized)
layout (location = 1) in vec3 aNormal;

//Colors
layout (location = 2) in vec3 aColor;

//Texture coords
layout (location = 3) in vec2 aTex;

out vec3 crntPos;
out vec3 Normal;
out vec3 color;
out vec2 texCoord;

uniform mat4 camMatrix;
uniform mat4 model; 

void main()
{
   crntPos = vec3(model * vec4(aPos,1.0f));
   gl_Position = camMatrix * vec4(crntPos,1.0);
   //Normal = aNormal;
   Normal = mat3(transpose(inverse(model))) * aNormal; //we scaled down the model that's why we must transpose and inverse
   color = aColor;
   texCoord = aTex;
}