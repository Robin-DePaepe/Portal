float4x4 gTransform : WORLDVIEWPROJECTION;
Texture2D gSpriteTexture;
float2 gTextureSize;

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

BlendState EnableBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};

RasterizerState BackCulling
{
	CullMode = BACK;
};

DepthStencilState depthStencilState
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

//SHADER STRUCTS
//**************
struct VS_DATA
{
	int Channel : TEXCOORD2; //Texture Channel
	float3 Position : POSITION; //Left-Top Character Quad Starting Position
	float4 Color: COLOR; //Color of the vertex
	float2 TexCoord: TEXCOORD0; //Left-Top Character Texture Coordinate on Texture
	float2 CharSize: TEXCOORD1; //Size of the character (in screenspace)
};

struct GS_DATA
{
	float4 Position : SV_POSITION; //HOMOGENEOUS clipping space position
	float4 Color: COLOR; //Color of the vertex
	float2 TexCoord: TEXCOORD0; //Texcoord of the vertex
	int Channel : TEXCOORD1; //Channel of the vertex
};

//VERTEX SHADER
//*************
VS_DATA MainVS(VS_DATA input)
{
	return input;
}

//GEOMETRY SHADER
//***************
void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float4 col, float2 texCoord, int channel)
{
	//Create a new GS_DATA object
	GS_DATA vertex = (GS_DATA)0;

	//Fill in all the fields

	vertex.Position = mul(float4(pos, 1.0f), gTransform);
	vertex.Color = col;
	vertex.TexCoord = texCoord;
	vertex.Channel = channel;

	//Append it to the TriangleStream
	triStream.Append(vertex);
}

[maxvertexcount(4)]
void MainGS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	//Create a Quad using the character information of the given vertex
	//Note that the Vertex.CharSize is in screenspace, TextureCoordinates aren't ;) [Range 0 > 1]
	float3 position = vertex[0].Position;
	float2 texCoord = vertex[0].TexCoord;

	//1. Vertex Left-Top
	CreateVertex(triStream, position, vertex[0].Color, texCoord, vertex[0].Channel);

	//2. Vertex Right-Top
	texCoord.x += vertex[0].CharSize.x / gTextureSize.x;
	position.x += vertex[0].CharSize.x;
	CreateVertex(triStream, position, vertex[0].Color, texCoord, vertex[0].Channel);

	//3. Vertex Left-Bottom
	texCoord.x -= vertex[0].CharSize.x / gTextureSize.x;
	position.x -= vertex[0].CharSize.x;

	position.y += vertex[0].CharSize.y;
	texCoord.y += vertex[0].CharSize.y / gTextureSize.y;

	CreateVertex(triStream, position, vertex[0].Color, texCoord, vertex[0].Channel);

	//4. Vertex Right-Bottom
	position.x += vertex[0].CharSize.x;
	texCoord.x += vertex[0].CharSize.x / gTextureSize.x;
	CreateVertex(triStream, position, vertex[0].Color, texCoord, vertex[0].Channel);
}

//PIXEL SHADER
//************
float4 MainPS(GS_DATA input) : SV_TARGET{

	float4 finalColor = (float4)0;

	finalColor = gSpriteTexture.Sample(samPoint, input.TexCoord)[input.Channel] * input.Color;

	return finalColor;
}

// Default Technique
technique10 Default {

	pass p0 {
		SetRasterizerState(BackCulling);
		SetDepthStencilState(depthStencilState, 0);

		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(CompileShader(gs_4_0, MainGS()));
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
	}
}
