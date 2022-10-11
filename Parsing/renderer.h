// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#include <DirectXMath.h>
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

struct ConstantBuffer
{
	DirectX::XMMATRIX matrixOne;
	DirectX::XMMATRIX matrixTwo;
	DirectX::XMMATRIX matrixThree;
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
// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	GW::INPUT::GInput input;
	DirectX::XMMATRIX viewMatrix;
	high_resolution_clock::time_point startTime = high_resolution_clock::now();
	
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
		Vertex verts[104] = {
			 0
		};
		float yValue = -0.5;
		float xValue = -0.5;
		float value = 0;
		float value2 = 0;
		for (int i = 0; i < 104;)
		{
			if (i >= 52) {
				verts[i] = {(xValue + (float)(value2 / 25.0f)), -0.5, 0, 1 };
				verts[i + 1] = {(xValue + (float)(value2 / 25.0f)), 0.5, 0, 1 };
				i += 2;
				value2++;
			}
			else {
				verts[i] = { -0.5,(yValue + (float)(value / 25.0f)),0,1 };
				verts[i + 1] = { 0.5,(yValue + (float)(value / 25.0f)),0,1 };
				i += 2;
				value++;
			}
		}

		CD3D11_BUFFER_DESC cDesc(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&cDesc, nullptr, constantBuffer.GetAddressOf());
		
		D3D11_SUBRESOURCE_DATA bData = { verts, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeof(verts), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());


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
		DirectX::XMMATRIX m2;
		DirectX::XMMATRIX m3;
		DirectX::XMMATRIX m4;
		DirectX::XMMATRIX m5;
		DirectX::XMMATRIX m6;
	
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
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(-0.005f, 0, 0),viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			if (sKey > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0, 0, -0.005f), viewMatrix);
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
			}
			if (dKey > 0) {
				viewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
				viewMatrix = DirectX::XMMatrixMultiply( DirectX::XMMatrixTranslation(0.005f, 0, 0), viewMatrix);
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

		ConstantBuffer cbuffer = { m, viewMatrix , perspectiveMatrix};

		m2 = DirectX::XMMatrixIdentity();
		m2 = DirectX::XMMatrixMultiply(m2, DirectX::XMMatrixRotationX(-1.5708f));
		m2 = DirectX::XMMatrixMultiply(m2, DirectX::XMMatrixTranslation(0.0f, -0.5f, 0.0f));

		m3 = DirectX::XMMatrixIdentity();
		m3 = DirectX::XMMatrixMultiply(m3, DirectX::XMMatrixRotationX(0));
		m3 = DirectX::XMMatrixMultiply(m3, DirectX::XMMatrixTranslation(0.0f, 0, 0.5f));

		m4 = DirectX::XMMatrixIdentity();
		m4 = DirectX::XMMatrixMultiply(m4, DirectX::XMMatrixRotationX(3.14159f));
		m4 = DirectX::XMMatrixMultiply(m4, DirectX::XMMatrixTranslation(0.0f, 0, -0.5f));

		m5 = DirectX::XMMatrixIdentity();
		m5 = DirectX::XMMatrixMultiply(m5, DirectX::XMMatrixRotationY(-1.5708f));
		m5 = DirectX::XMMatrixMultiply(m5, DirectX::XMMatrixTranslation(-0.5f, 0, 0));

		m6 = DirectX::XMMatrixIdentity();
		m6 = DirectX::XMMatrixMultiply(m6, DirectX::XMMatrixRotationY(-1.5708f));
		m6 = DirectX::XMMatrixMultiply(m6, DirectX::XMMatrixTranslation(0.5f, 0, 0));



		// grab the context & render target
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;

		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		d3d.GetDepthStencilView((void**)&depth);
		// setup the pipeline
		ID3D11RenderTargetView *const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);
		const UINT strides[] = { sizeof(Vertex) };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
		con->VSSetShader(vertexShader.Get(), nullptr, 0);
		con->PSSetShader(pixelShader.Get(), nullptr, 0);
		con->IASetInputLayout(vertexFormat.Get());

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());


		// now we can draw
		con->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
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
		con->Draw(104, 0);


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
