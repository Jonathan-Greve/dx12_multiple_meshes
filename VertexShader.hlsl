// Explicit padding to an alignment of 256 bytes
// The padding can be removed for implicit padding
cbuffer cbPerObject : register(b0)
{
	float4x4 World;
	float4x4 Pad0;
	float4x4 Pad1;
	float4x4 Pad2;
};

cbuffer cbPerRenderPass : register(b1)
{
	float4x4 ViewProj;
	double DeltaTime;
	double TotalTime;
	double Pad3[6];
	float4x4 Pad4;
	float4x4 Pad5;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float4 Color : COLOR;
	float3 normalL : NORMAL;
	float3 tangentL : TANGENT;
	float2 texCord : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut main(VertexIn vIn)
{
	VertexOut vOut;

	// Transform to homogeneous clip space.
	vOut.PosH = mul(float4(vIn.PosL.x, vIn.PosL.y, vIn.PosL.z, 1.0f), World);
	vOut.PosH.y += sin(TotalTime) * 3;
	vOut.PosH = mul(vOut.PosH, ViewProj);
	//vOut.PosH = float4(-vIn.PosL.x-0.5, -vIn.PosL.y-0.5, 0.5, 1.0);

	vOut.Color = vIn.Color;
	vOut.Color.r *= sin(TotalTime*4) * 0.5 + 0.5;

	return vOut;
}