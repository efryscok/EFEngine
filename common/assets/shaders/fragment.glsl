#version 430

out vec4 colour;

in VS_OUT {
	vec4 normal;
	vec4 texCoord;
	vec3 L;
	vec3 V;
	vec3 vertWorld;
} fs_in;

uniform bool useLight;
uniform bool isSkybox;
uniform vec3 cameraPosition;
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform float alpha;
uniform sampler2D skybox;

struct LIGHT_DESCRIPTION {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float attenConst;
	float attenLinear;
	float attenQuad;
};

const int NUMBER_OF_LIGHTS = 1;
uniform LIGHT_DESCRIPTION lights[NUMBER_OF_LIGHTS];

vec3 addLight(in LIGHT_DESCRIPTION light) {
	vec3 vNormal = normalize(fs_in.normal.xyz);
	vec3 vLight = normalize(light.position - fs_in.vertWorld);
	vec3 vView = normalize(cameraPosition - fs_in.vertWorld);
	vec3 vReflect = -(reflect(vLight, vNormal));
	vec3 vAmbient = materialAmbient * light.ambient;
	vec3 vDiffuse = max(0.f, dot(vLight, vNormal)) * materialDiffuse * light.diffuse;
	float fDistance = distance(fs_in.vertWorld, light.position);
	float fAttenuation = 1.f / (
		light.attenConst + 
		light.attenLinear * fDistance + 
		light.attenQuad * fDistance * fDistance
	);
	vDiffuse *= fAttenuation;
	vec3 vSpecular = vec3(0.f);
	if (dot(vLight, vNormal) > 0.f) {
		vSpecular = pow(max(0.f, dot(vView, vReflect)), materialShininess) * materialSpecular * light.specular;
		vSpecular *= fAttenuation;
	}
	return clamp(vAmbient + vDiffuse + vSpecular, 0.f, 1.f);
}

void main(void) {
	if (isSkybox) {
		colour = texture(skybox, fs_in.texCoord.xy);
	}
	else {
		vec3 lightCalc = vec3(0.f);
		
		if (useLight) {
			for (int i = 0; i < NUMBER_OF_LIGHTS; ++i) {
				lightCalc += addLight(lights[i]);
			}
		}
		else {
			lightCalc = materialDiffuse;
		}
		
		colour = vec4(lightCalc, alpha);
	}
}