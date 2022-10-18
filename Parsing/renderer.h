// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include "Level.h"
#include <windows.h>
#include <shobjidl.h> 
#pragma comment(lib, "d3dcompiler.lib")
// Simple Vertex Shader
// Simple Pixel Shader


struct ConstantWorldBuffer
{
	DirectX::XMFLOAT4X4 world;
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
void ParseFile(const char* filename, std::vector< DirectX::XMFLOAT4X4>& matrixVect, std::vector<Model>& modelsVec, DirectX::XMMATRIX& viewMatrix);
// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vpConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		colorBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
	ID3D11Device* creator;

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

		d3d.GetDevice((void**)&creator);
		input.Create(_win);

		// Create Vertex Buffer
		ParseFile(filename, matrixVect, modelVec, viewMatrix);

		//if (viewMatrix == DirectX::empty) {
		//}
		for (int i = 0; i < modelVec.size(); i++)
		{
			modelVec[i].CreateBuffer(creator, d3d);
		}

		CD3D11_BUFFER_DESC cDesc(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&cDesc, nullptr, vpConstantBuffer.GetAddressOf());

		CD3D11_BUFFER_DESC sbDesc(sizeof(ColorBuff), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&cDesc, nullptr, colorBuffer.GetAddressOf());

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
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},{

				"TEXTCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},{

				"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
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
#pragma region LevelSwap
		float fOneKey;
		input.GetState(G_KEY_F1, fOneKey);

		if (fOneKey > 0) {
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
				COINIT_DISABLE_OLE1DDE);
			if (SUCCEEDED(hr))
			{
				IFileOpenDialog* pFileOpen;
				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
					IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

				if (SUCCEEDED(hr))
				{
					// Show the Open dialog box.
					hr = pFileOpen->Show(NULL);
					// Get the file name from the dialog box.
					if (SUCCEEDED(hr))
					{
						IShellItem* pItem;
						hr = pFileOpen->GetResult(&pItem);
						if (SUCCEEDED(hr))
						{
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
							matrixVect.clear();
							modelVec.clear();
							std::wstring convert = pszFilePath;
							std::string temp = std::string(convert.begin(), convert.end());
							filename = temp.c_str();
							ParseFile(filename, matrixVect, modelVec, viewMatrix);
							for (int i = 0; i < modelVec.size(); i++)
							{
								modelVec[i].CreateBuffer(creator, d3d);
							}
							// Display the file name to the user.
							pItem->Release();
						}
					}
					pFileOpen->Release();
				}
				CoUninitialize();
			}

		}
#pragma endregion

#pragma region Keyboard Input
		float wKey, aKey, sKey, dKey, space, shift;
		float mouseX, mouseY;
		float movementSpeed = 0.20f;
		unsigned int  height, width;
		win.GetHeight(height);
		win.GetWidth(width);
		input.GetState(G_KEY_W, wKey);
		input.GetState(G_KEY_A, aKey);
		input.GetState(G_KEY_S, sKey);
		input.GetState(G_KEY_D, dKey);
		input.GetState(G_KEY_LEFTSHIFT, shift);
		input.GetState(G_KEY_SPACE, space);
		GW::GReturn result = input.GetMouseDelta(mouseX, mouseY);
		mouseX = ((65 * 3.14) / 180) * mouseX / (float)width;
		mouseY = ((65 * 3.14) / 180) * mouseY / (float)height;
		high_resolution_clock::time_point currTime = high_resolution_clock::now();
		duration<double> timespan = duration_cast<duration<double>>(currTime - startTime);
		if (timespan.count() > 0.0167) {
			if (result == GW::GReturn::SUCCESS) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(mouseY), viewMatrix);
				DirectX::XMVECTOR pos = viewMatrix.r[3];
				viewMatrix = DirectX::XMMatrixMultiply(viewMatrix, DirectX::XMMatrixRotationY(mouseX));
				viewMatrix.r[3] = pos;
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			else {
				mouseX = 0;
				mouseY = 0;
			}
			if (wKey > 0 || aKey > 0 || sKey > 0 || dKey > 0 || shift > 0 || space > 0)
			{
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				if (wKey > 0) {
					viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, movementSpeed), viewMatrix);
				}
				if (aKey > 0) {
					viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(-movementSpeed, 0, 0), viewMatrix);
				}
				if (sKey > 0) {
					viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -movementSpeed), viewMatrix);
				}
				if (dKey > 0) {
					viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(movementSpeed, 0, 0), viewMatrix);
				}
				if (shift > 0) {
					viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, -movementSpeed, 0), viewMatrix);
				}
				if (space > 0) {
					viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, movementSpeed, 0), viewMatrix);
				}
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			startTime = high_resolution_clock::now();
		}

#pragma endregion

		float lightDir[4] = { -1,-1,2,0 };
		float lightColor[4] = { 0.9f, 0.9f, 1.0f, 1.0f };
		float cameraWorldPos[4] = { 0.0f, 0.0f, -13.0f, 1.0 };
		float ambientLight[4] = { 0.25, 0.25, 0.35, 1 };

		float aRatio;
		d3d.GetAspectRatio(aRatio);

		DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH(1.13446f, aRatio, 0.1f, 100.0f);

		ConstantBuffer cbuffer = { viewMatrix , perspectiveMatrix };

		// grab the context & render target
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;

		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		d3d.GetDepthStencilView((void**)&depth);

		con->VSSetConstantBuffers(1, 1, vpConstantBuffer.GetAddressOf());
		con->PSSetConstantBuffers(1, 1, vpConstantBuffer.GetAddressOf());
		con->UpdateSubresource(vpConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);

		con->PSSetConstantBuffers(1, 1, colorBuffer.GetAddressOf());
		con->UpdateSubresource(colorBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);

		con->VSSetShader(vertexShader.Get(), nullptr, 0);
		con->PSSetShader(pixelShader.Get(), nullptr, 0);
		con->IASetInputLayout(vertexFormat.Get());

		ColorBuff colorBuff = { 0 };
		for (int i = 0; i < 4; i++)
		{
			colorBuff.lightDir[i] = lightDir[i];
			colorBuff.lightColor[i] = lightColor[i];
			colorBuff.camPos[i] = cameraWorldPos[i];
			colorBuff.ambient[i] = ambientLight[i];

		}

		for (int i = 0; i < modelVec.size(); i++)
		{
			modelVec[i].LoadModel(&modelVec[i], &d3d, con, view, depth, colorBuffer, &colorBuff);
		}

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

void ParseFile(const char* filename, std::vector< DirectX::XMFLOAT4X4>& _matrixVect, std::vector<Model>& modelsVec, DirectX::XMMATRIX& _view) {

	std::ifstream file;
	DirectX::XMFLOAT4X4 objMatrix;	//matrix that holds values from each amtrix in the text file
	float objVect[4];				//vector of collect row values from the matrix
	std::string valueString;		//string of floats for each number in matrix
	std::string character;			//each character in the matrix string 
	float value;					//stored value of string after stof
	int count = 0;					//vector counter
	int count2 = 0;	
	DirectX::FXMVECTOR eye{ 0.0f, 0.0f, -13.0f };
	DirectX::FXMVECTOR focus{ 0,0.5,0 };
	DirectX::FXMVECTOR upDir{ 0,1,0 };
	_view = DirectX::XMMatrixLookAtLH(eye, focus, upDir);
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

		if (std::strcmp(output.c_str(), "MESH") == 0) {

			std::getline(file, h2b, '\n');
			size_t found = h2b.find_last_of('.', h2b.size());
			if (found != std::string::npos) {
				h2b.resize(h2b.length() - (h2b.length() - found));
			}
			h2b = "../TriangulatedBlenderAssets/" + h2b;
			bool val;
			h2b.append(".h2b");
			Model models;
			val = models.parser.Parse(h2b.c_str());
			if (val == true) {
				modelsVec.push_back(models);
			}

			std::getline(file, matrix, '(');
			std::getline(file, matrix, '>');
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
							_matrixVect.push_back(objMatrix);
							count2 = 0;
						}
					}
					valueString = "";
				}
			}
		}
		else if (std::strcmp(output.c_str(), "CAMERA") == 0) {
			std::getline(file, h2b, '\n');
			std::getline(file, matrix, '(');
			std::getline(file, matrix, '>');
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
							_view.r[0] = DirectX::XMVECTOR{ objMatrix.m[0][0],objMatrix.m[0][1], objMatrix.m[0][2], objMatrix.m[0][3] };
							_view.r[1] = DirectX::XMVECTOR{ objMatrix.m[1][0],objMatrix.m[1][1], objMatrix.m[1][2], objMatrix.m[1][3] };
							_view.r[2] = DirectX::XMVECTOR{ objMatrix.m[2][0],objMatrix.m[2][1], objMatrix.m[2][2], objMatrix.m[2][3] };
							_view.r[3] = DirectX::XMVECTOR{ objMatrix.m[3][0],objMatrix.m[3][1], objMatrix.m[3][2], objMatrix.m[3][3] };
							count2 = 0;

							_view = DirectX::XMMatrixInverse(nullptr, _view);
						}
					}
					valueString = "";
				}
			}
		}
		else if (std::strcmp(output.c_str(), "LIGHT") == 0)
		{

		}
	}
	file.close();
	for (int i = 0; i < _matrixVect.size(); i++)
	{
		modelsVec[i].modelWoldStructs.world = _matrixVect[i];
	}
}
