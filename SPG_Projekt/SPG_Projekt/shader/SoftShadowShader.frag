#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

int shadowSize = 1024;

float blur(vec2 fragCoord, float c)
{
    float Pi = 6.28318530718; // Pi*2
    
    // GAUSSIAN BLUR SETTINGS {{{
    float Directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 5.0; // BLUR QUALITY (Default 4.0 - More is better but slower)
    float Size = 8.0; // BLUR SIZE (Radius)
    // GAUSSIAN BLUR SETTINGS }}}
   
    vec2 Radius = vec2(Size/shadowSize);
    
    // Pixel colour
    float Color = texture(shadowMap, fragCoord).r;
    
    // Blur calculations
    for( float d=0.0; d<Pi; d+=Pi/Directions)
    {
		for(float i=1.0/Quality; i<=1.0; i+=1.0/Quality)
        {
			float thisVal = texture( shadowMap, fragCoord+vec2(cos(d),sin(d))*Radius*i).r;	
			Color += exp(c*thisVal);
        }
    }
    
    // Output to screen
    Color /= Quality * Directions - 15.0;
    return Color;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.007);
	//float bias = 0.007;
	float c = 30;
	float blurredExpZ = blur(projCoords.xy, c);
	float shadow = clamp(1 - exp(-c*currentDepth)*blurredExpZ, 0, 1);
	return shadow;
}


void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    //vec3 color = vec3(1.0, 1.0, 0.0);
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);                    
    //float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                    
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}