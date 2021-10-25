//Matrices
float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;

//Portal
Texture2D gPortalTexture;
bool gIsBlue;
bool gIsActive;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = NONE;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float4 TexCoord2 : TEXCOORD2;
	float3 Normal : NORMAL;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
	output.TexCoord = input.TexCoord;
	output.Normal = mul(input.Normal, (float3x3)gWorld);

	output.TexCoord2 = output.Position;

	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
	float2 center = float2(0.5f,0.5f); //text coord center

	input.TexCoord2.xy = (float2(1, -1) * input.TexCoord2.xy / input.TexCoord2.w * 0.5f + 0.5f);

	if (sqrt(pow(input.TexCoord.x - center.x, 2) + pow(input.TexCoord.y - center.y, 2)) <= 0.48f)
		{
		if (gIsActive)
		{
			float4 finalColor = gPortalTexture.Sample(samLinear, input.TexCoord2.xy);
			finalColor.w = 1.f;
			return finalColor;
		}
			else return  float4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	//give the edge the color of the portal
	else if (sqrt(pow(input.TexCoord.x - center.x, 2) + pow(input.TexCoord.y - center.y, 2)) <= 0.5f)
	{
		if (gIsBlue) return float4(0.0f, 0.0f, 1.0f, 1.0f);
		return float4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	return  float4(0.0f, 0.0f, 0.0f, 0.0f);
}

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};


//TECHNIQUE
//---------
technique11  PortalShader
{
	pass P0
	{
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
	}
}