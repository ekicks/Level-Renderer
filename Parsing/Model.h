#pragma once
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "h2bParser.h"
#include "Gateware.h"
#include <string.h>
#include <wrl/client.h>
#include <d3d11.h>

struct LightVector
{
	float x, y, z, w;
};
struct ColorBuff
{
	LightVector lightDir;
	LightVector lightColor;
	LightVector lightPos;

	LightVector lightDirVec[8];
	LightVector lightColorVec[8];
	LightVector lightPosVec[8];
	H2B::ATTRIBUTES outputColor;

	float camPos[4];
	float ambient[4];
	int lightCount;
	int padding[2];
};

class Model
{
	
public:

	struct ConstWorld
	{
		DirectX::XMFLOAT4X4 world;	//world matrix array for different material
	};

	H2B::Parser parser;
	Model::ConstWorld modelWoldStructs;
	H2B::ATTRIBUTES attributes;

	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	//void Model::ParseFile(const char* filename, std::vector< DirectX::XMFLOAT4X4> matrixVect, std::vector<H2B::Parser> parserVec, std::vector<Model> modelsVec); //parse game level file
	void SetData(Model& model, GW::GRAPHICS::GDirectX11Surface d3d, ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ID3D11DepthStencilView* depth);
	void CreateBuffer(ID3D11Device* creator, GW::GRAPHICS::GDirectX11Surface _d3d);
	void LoadModel(Model* model, GW::GRAPHICS::GDirectX11Surface* d3d, ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ID3D11DepthStencilView* depth, Microsoft::WRL::ComPtr<ID3D11Buffer> colorBuffer, ColorBuff* colorBuff);
	Model();
	~Model();
	
};

