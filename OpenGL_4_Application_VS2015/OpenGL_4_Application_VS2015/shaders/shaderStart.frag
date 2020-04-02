#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;
in vec2 fragTexCoords;
in vec4 vPos;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform	vec3 lightColor;
uniform	vec3 lightDir;
uniform vec3 pointLight;
uniform vec3 pointLightColor;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform float diffuseStregth;
uniform float ambientStrength;
uniform float specularStrength;
uniform int isTree;

vec3 ambient;
vec3 diffuse;
vec3 specular;
vec3 pAmbient;
vec3 pDiffuse;
vec3 pSpecular;
vec4 fogColor;
float shininess = 64.0f;

float constant = 1.0f;
float linear = 0.35f;
float quad = 0.44f;
float att = 1.0f;
float pointLightDist = 0.0f;

vec3 o;

float computeFog(){
	float fragmentDistance;
 	float fogDensity = 0.05f;
 	if(diffuseStregth == 0.0f){
 		fragmentDistance = length(fragPosEye)/1;
 	}else{
 		fragmentDistance = length(fragPosEye)/3;
 	}
 	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 	return clamp(fogFactor, 0.0f, 1.0f);
}

void computeLightComponents()
{	

	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor * diffuseStregth;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

void computePointLightComponents()
{	
	pointLightDist =  length(pointLight - vPos.xyz);
	att =  1.0f / (constant + linear * pointLightDist + quad * (pointLightDist * pointLightDist));

	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(pointLight - vPos.xyz);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - vPos.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	pAmbient = att*  pointLightColor;
	
	//compute diffuse light
	pDiffuse = att * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	pSpecular = att *  specCoeff * pointLightColor * specularStrength * shininess;
}

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

vec4 colorFromTexture;
void main() 
{
	computeLightComponents();
	computePointLightComponents();
	float shadow = computeShadow();
	
	colorFromTexture = texture(diffuseTexture, fragTexCoords);
	if( (isTree == 1) && (colorFromTexture.a < 0.1) ) {
		discard;
	}
	//modulate with diffuse map
	ambient *= vec3(colorFromTexture.xyz);
	diffuse *= vec3(colorFromTexture.xyz);
	//modulate woth specular map
	specular *= vec3(colorFromTexture.xyz);

	pAmbient *= vec3(colorFromTexture.xyz);
	pDiffuse *= vec3(colorFromTexture.xyz);
	pSpecular *= vec3(colorFromTexture.xyz);
	
	//modulate with shadow
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	vec4 v4color = vec4(color, 1.0f);
	vec3 pColor = min((pAmbient + (1.0f - shadow)*pDiffuse) + (1.0f - shadow)*pSpecular, 1.0f);
	vec4 Pv4Color =  vec4(pColor, 1.0f);

	float fogFactor = computeFog();

	if(diffuseStregth == 0.0f){
		fogColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}else{
		fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	}
	
	fColor = mix(fogColor, v4color, fogFactor) + Pv4Color;
}
