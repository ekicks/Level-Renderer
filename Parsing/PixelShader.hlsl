struct OBJ_ATTRIBUTES
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    unsigned int illum; // illumination model
};

cbuffer ConstWorld : register(b0)
{
    float4x4 world[1];
};
cbuffer ConstBuff : register(b1)
{
    float4x4 view;
    float4x4 projection;
};
cbuffer ColorBuff : register(b2)
{
    float4 lightDir;
    float4 lightColor;
    OBJ_ATTRIBUTES outputColor;
	
    float4 camPos;
    float4 ambient;
};

struct pixel
{
    float4 pos : SV_POSITION;
    float4 nrm : COLOR;
    float4 wPos : WORLD;
};

float4 main(pixel inputPS) : SV_TARGET
{
    float3 normal = normalize(inputPS.nrm.xyz);
    float lightRatio = saturate(dot(-lightDir.xyz, normal));
	
   float4 color = float4(outputColor.Kd, 1);
	
    float3 viewDir = normalize(camPos.xyz - inputPS.wPos.xyz);
    float3 halfVec = normalize(reflect(lightDir.xyz, normal));

    float intensity = max(pow(saturate(dot(viewDir, halfVec)), outputColor.Ns), 0);
    float4 reflectedLight = float4(outputColor.Ks, 1) * 1 * intensity;
	
    return saturate(lightRatio * lightColor + ambient) * color + reflectedLight;
}