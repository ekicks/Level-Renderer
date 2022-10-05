// Simple basecode showing how to create a window and attatch a d3d11surface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this
// With what we want & what we don't defined we can include the API
#define GATEWARE_ENABLE_INPUT
#include "Gateware.h"
#include "renderer.h" // example rendering code (not Gateware code!)
#include "h2bParser.h"
#include <string.h>
// open some namespaces to compact the code a bit
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;
// lets pop a window and use D3D11 to clear to a green screen
void parseFile();

int main()
{
	parseFile();
	GWindow win;
	GEventResponder msgs;
	GDirectX11Surface d3d11;
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		float clr[] = { 57/255.0f, 1.0f, 20/255.0f, 1 }; // start with a neon green
		msgs.Create([&](const GW::GEvent& e) {
			GW::SYSTEM::GWindow::Events q;
			if (+e.Read(q) && q == GWindow::Events::RESIZE)
				clr[2] += 0.01f; // move towards a cyan as they resize
		});
		win.Register(msgs);
		if (+d3d11.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		{
			Renderer renderer(win, d3d11);
			while (+win.ProcessWindowEvents())
			{
				IDXGISwapChain* swap;
				ID3D11DeviceContext* con;
				ID3D11RenderTargetView* view;
				ID3D11DepthStencilView* depth;
				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetDepthStencilView((void**)&depth) &&
					+d3d11.GetSwapchain((void**)&swap))
				{
					con->ClearRenderTargetView(view, clr);
					con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);
					renderer.Render();
					swap->Present(1, 0);
					// release incremented COM reference counts
					swap->Release();
					view->Release();
					depth->Release();
					con->Release();
				}
			}
		}
	}
	return 0; // that's all folks
}

void parseFile() {
	std::ifstream file;
	DirectX::XMFLOAT4X4 objMatrix;
	float objVect[4];
	std::vector< DirectX::XMFLOAT4X4> matrixVect;
	std::string valueString;
	std::string lastCharacter;
	std::string character;
	float value;
	int count =0;
	int count2 =0;

	file.open("../GameLevel.txt");
	while (true) {
		std::string output = "";
		H2B::Parser parser;
		std::string h2b = "";
		std::string matrix = "";
		

		if (file.eof() == true) {
			break;
		}

		std::getline(file, output, '\n');

		if (std::strcmp(output.c_str(), "MESH") == 0 || std::strcmp(output.c_str(), "LIGHT") == 0 || std::strcmp(output.c_str(), "CAMERA") == 0) {
			std::cout << output << std::endl;
			std::getline(file, h2b, '\n');
			std::cout << h2b << std::endl;
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