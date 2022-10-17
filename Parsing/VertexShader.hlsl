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

cbuffer ConstWorld : register (b0)
{
    float4x4 world[1];
};
cbuffer ConstBuff : register(b1)
{
    matrix view;
    matrix projection;
};

struct vert
{
    float3 pos : POSITION;
    float3 uvw : TEXTCOORD;
    float3 nrm : COLOR;
};
struct pixel
{
    float4 pos : SV_POSITION;
    float4 nrm : COLOR;
    float4 wPos : WORLD;
};

pixel main(vert inputVertex)
{
    pixel pixelOutput = (pixel) 0;
    pixelOutput.nrm = mul(world[0], float4(inputVertex.nrm, 0));
    //pixelOutput.pos = mul(view, float4(inputVertex.pos, 1));
  
    pixelOutput.pos = mul(world[0], float4(inputVertex.pos, 1));
    pixelOutput.wPos = pixelOutput.pos;
    pixelOutput.pos = mul(view, pixelOutput.pos);
    pixelOutput.pos = mul(projection, pixelOutput.pos);
    
    return pixelOutput;
}