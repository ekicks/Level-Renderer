cbuffer ConstantBuffer 
{
    matrix matrixOne;
    matrix matrixTwo;
    matrix matrixThree;
};
float4 main(float4 inputVertex : POSITION) : SV_POSITION
{
    float4 worldPos = mul(matrixOne, inputVertex);
    float4 viewPos = mul(matrixTwo, worldPos);
    float4 perspective = mul(matrixThree, viewPos);
    return perspective;
}