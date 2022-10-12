#pragma once
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "h2bParser.h"
#include "Gateware.h"
#include <string.h>
#include <wrl/client.h>
#include <d3d11.h>

class Model
{
	
public:
	struct ConstWorld
	{
		DirectX::XMFLOAT4X4 world[256];	//world matrix array for different material
	};

	struct OBJ_ATTRIBUTES
	{
		float Kd[3]; // diffuse reflectivity
		float d; // dissolve (transparency) 
		float Ks[3]; // specular reflectivity
		float Ns; // specular exponent
		float Ka[3]; // ambient reflectivity
		float sharpness; // local reflection map sharpness
		float Tf[3]; // transmission filter
		float Ni; // optical density (index of refraction)
		float Ke[3]; // emissive reflectivity
		unsigned int illum; // illumination model
	};

	H2B::Parser parsed;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	void ParseFile(const char* filename, std::vector< DirectX::XMFLOAT4X4> matrixVect); //parse game level file
	void SetData(Model& model, GW::GRAPHICS::GDirectX11Surface d3d);
	
};

