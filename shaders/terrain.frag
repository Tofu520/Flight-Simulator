#version 330 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

uniform sampler2D tex0;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

void main()
{
    vec3 normal   = normalize(Normal);
    vec3 lightDir = normalize(lightPos);
    vec3 viewDir  = normalize(camPos - crntPos);

    float ambient  = 0.35;
    float diffuse  = max(dot(normal, lightDir), 0.0);
    float spec     = pow(max(dot(viewDir, reflect(-lightDir, normal)), 0.0), 32.0);
    float specular = 0.25 * spec;

    vec4 texColor = texture(tex0, texCoord);

    vec4 litColor = texColor * (ambient + diffuse + specular) * lightColor;

    float dist      = length(camPos - crntPos);
    float fogFactor = 1.0 - clamp((dist - 4000.0) / 5000.0, 0.0, 1.0);
    vec4  fogColor  = vec4(0.53, 0.81, 0.98, 1.0);

    FragColor = mix(fogColor, litColor, fogFactor);
}
