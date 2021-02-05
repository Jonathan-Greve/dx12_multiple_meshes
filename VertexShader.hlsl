// Explicit padding to an alignment of 256 bytes
// The padding can be removed for implicit padding
cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float4x4 Pad0;
	float4x4 Pad1;
	float4x4 Pad2;
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
	vOut.PosH = mul(float4(vIn.PosL.x, vIn.PosL.y, vIn.PosL.z, 1.0f), gWorldViewProj);
	//vOut.PosH = float4(-vIn.PosL.x-0.5, -vIn.PosL.y-0.5, 0.5, 1.0);

	vOut.Color = vIn.Color;

	return vOut;
}