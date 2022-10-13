// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include "Level.h"
#pragma comment(lib, "d3dcompiler.lib")
// Simple Vertex Shader
// Simple Pixel Shader

struct Vertex
{
	float x;
	float y;
	float z;
	float w;
};

struct ConstantWorldBuffer
{
	DirectX::XMFLOAT4X4 world;
	int meshId;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};


std::string ShaderAsString(const char* shaderFilePath) {
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	else
		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
	return output;
}


using namespace std::chrono;
void ParseFile(const char* filename, std::vector< DirectX::XMFLOAT4X4>& matrixVect, std::vector<Model>& modelsVec);
// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	GW::INPUT::GInput input;
	DirectX::XMMATRIX viewMatrix;
	high_resolution_clock::time_point startTime = high_resolution_clock::now();

	std::vector<Model> modelVec;
	std::vector< DirectX::XMFLOAT4X4> matrixVect;
	const char* filename = "../GameLevel.txt";
public:

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);
		input.Create(_win);

		DirectX::FXMVECTOR eye{ 0.25f, -0.125f, -0.25f };
		DirectX::FXMVECTOR focus{ 0,-0.5,0 };
		DirectX::FXMVECTOR upDir{ 0,1,0 };

		viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, upDir);

		// Create Vertex Buffer
		
		// -------------- Replace surrounded -------------- //
		ParseFile(filename, matrixVect,modelVec);
		for (int i = 0; i < modelVec.size(); i++)
		{
			modelVec[i].CreateBuffer(creator, d3d);
		}
		// -------------- Replace surrounded -------------- //
		//Create Data


		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif

		std::string VS = ShaderAsString("../VertexShader.hlsl");
		std::string PS = ShaderAsString("../PixelShader.hlsl");


		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;
		if (SUCCEEDED(D3DCompile(VS.c_str(), strlen(VS.c_str()),
			nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
			vsBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
		// Create Pixel Shader
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (SUCCEEDED(D3DCompile(PS.c_str(), strlen(PS.c_str()),
			nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
			psBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
		// Create Input Layout
		D3D11_INPUT_ELEMENT_DESC format[] = {
			{
				"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			}
		};
		creator->CreateInputLayout(format, ARRAYSIZE(format),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
		// free temporary handle
		creator->Release();
	}
	void Render()
	{
		DirectX::XMMATRIX m;

		m = DirectX::XMMatrixIdentity();
		m = DirectX::XMMatrixMultiply(m, DirectX::XMMatrixRotationX(1.5708f));
		m = DirectX::XMMatrixMultiply(m, DirectX::XMMatrixTranslation(0.0f, 0.5f, 0.0f));
		float wKey, aKey, sKey, dKey, space, shift;
		float mouseX, mouseY;
		input.GetState(G_KEY_W, wKey);
		input.GetState(G_KEY_A, aKey);
		input.GetState(G_KEY_S, sKey);
		input.GetState(G_KEY_D, dKey);
		input.GetState(G_KEY_LEFTSHIFT, shift);
		input.GetState(G_KEY_SPACE, space);
		GW::GReturn result = input.GetMouseDelta(mouseX, mouseY);
		high_resolution_clock::time_point currTime = high_resolution_clock::now();
		duration<double> timespan = duration_cast<duration<double>>(currTime - startTime);
		if (timespan.count() > 0.0167) {
			if (result == GW::GReturn::SUCCESS) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(mouseX / 360.0f), viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(mouseY / 360.0f), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			else {
				mouseX = 0;
				mouseY = 0;
			}
			if (wKey > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, 0.005f), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			if (aKey > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(-0.005f, 0, 0), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			if (sKey > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -0.005f), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			if (dKey > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0.005f, 0, 0), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			if (shift > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, -0.005f, 0), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			if (space > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0.005f, 0), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			startTime = high_resolution_clock::now();
		}


		float aRatio;
		d3d.GetAspectRatio(aRatio);

		DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH(1.13446f, aRatio, 0.1f, 100.0f);

		ConstantBuffer cbuffer = { viewMatrix , perspectiveMatrix };
		ConstantWorldBuffer cwbuffer = { matrixVect[5] };

		// grab the context & render target
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;

		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		d3d.GetDepthStencilView((void**)&depth);
		
		modelVec[5].SetData(modelVec[5], d3d, con, view, depth);
		con->UpdateSubresource(modelVec[5].constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		con->VSSetConstantBuffers(0,1,modelVec[5].constantBuffer.GetAddressOf());
		con->PSSetConstantBuffers(0,1,modelVec[5].constantBuffer.GetAddressOf());
		con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		con->DrawIndexed(modelVec[5].parser.indexCount, 0, 1);


		// -------------- Replace surrounded -------------- //
		/*con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		con->Draw(104, 0);

		cbuffer.matrixOne = m2;
		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		con->Draw(104, 0);

		cbuffer.matrixOne = m3;
		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		con->Draw(104, 0);

		cbuffer.matrixOne = m4;
		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		con->Draw(104, 0);

		cbuffer.matrixOne = m5;
		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		con->Draw(104, 0);

		cbuffer.matrixOne = m6;
		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		con->Draw(104, 0);*/
		// -------------- Replace surrounded -------------- //

		//DrawModel || Load Level



		// release temp handles
		depth->Release();
		view->Release();
		con->Release();
	}
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}


};

void ParseFile(const char* filename, std::vector< DirectX::XMFLOAT4X4>& matrixVect, std::vector<Model>& modelsVec) {

	std::ifstream file;
	DirectX::XMFLOAT4X4 objMatrix;	//matrix that holds values from each amtrix in the text file
	float objVect[4];				//vector of collect row values from the matrix
	std::string valueString;		//string of floats for each number in matrix
	std::string character;			//each character in the matrix string 
	float value;					//stored value of string after stof
	int count = 0;					//vector counter
	int count2 = 0;					//matrix counter

	file.open(filename);
	while (true) {
		std::string output = "";
		H2B::Parser parser;
		std::string h2b = "";
		std::string matrix = "";

		if (file.eof() == true) {	//exits out the loop if end of file is met 
			break;
		}

		std::getline(file, output, '\n');	//gets the type of object

		if (std::strcmp(output.c_str(), "MESH") == 0 || std::strcmp(output.c_str(), "LIGHT") == 0 || std::strcmp(output.c_str(), "CAMERA") == 0) {

			std::cout << output << std::endl; //prints the name of the object 

			std::getline(file, h2b, '\n');
			std::cout << h2b << std::endl;
			size_t found = h2b.find_last_of('.', h2b.size());
			if (found != std::string::npos) {
				h2b.resize(h2b.length() - (h2b.length() - found));
			}
			h2b = "../Assets/" + h2b;
			bool val;
			h2b.append(".h2b");
			Model models;
			val = models.parser.Parse(h2b.c_str());
			modelsVec.push_back(models);

			std::getline(file, matrix, '(');
			std::cout << matrix << "(";
			std::getline(file, matrix, '>');
			std::cout << matrix << ">" << std::endl;
			for (int i = 0; i < matrix.length(); i++)
			{
				character = matrix[i];
				if ((character[0] >= 48 && character[0] <= 57 || character[0] == 46 || character[0] == 45)) {
					valueString.append(character);
				}
				if (matrix[i] == ',' || matrix[i] == ')') {

					value = std::stof(valueString);
					objVect[count] = value;
					count++;
					if (count == 4) {
						count = 0;
						objMatrix.m[count2][0] = objVect[0];
						objMatrix.m[count2][1] = objVect[1];
						objMatrix.m[count2][2] = objVect[2];
						objMatrix.m[count2][3] = objVect[3];
						count2++;
						if (count2 == 4) {
							count2 = 0;
						}
						matrixVect.push_back(objMatrix);
					}
					valueString = "";
				}
			}
		}

	}
	file.close();
}

