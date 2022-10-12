#include "Model.h"

void ParseFile(const char* filename, std::vector< DirectX::XMFLOAT4X4> matrixVect) {

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
				h2b.resize(h2b.length() - found);
			}
			h2b = "../Assets/" + h2b;
			h2b.append(".h2b");
			H2B::Parser parseh2b;
			bool val;
			val = parseh2b.Parse(h2b.c_str());

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


void Model::SetData(Model& model, GW::GRAPHICS::GDirectX11Surface d3d) {

	ID3D11DeviceContext* con;
	ID3D11RenderTargetView* view;
	ID3D11DepthStencilView* depth;
	d3d.GetImmediateContext((void**)&con);
	d3d.GetRenderTargetView((void**)&view);
	d3d.GetDepthStencilView((void**)&depth);
	// setup the pipeline
	ID3D11RenderTargetView* const views[] = { view };
	con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);
	const UINT strides[] = { sizeof(Model::ConstWorld) };
	const UINT offsets[] = { 0 };
	ID3D11Buffer* const buffs[] = { model.vertexBuffer.Get()};
	con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	con->IASetIndexBuffer(model.indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	con->VSSetConstantBuffers(0, 1, model.constantBuffer.GetAddressOf());
	con->PSSetConstantBuffers(0, 1, model.constantBuffer.GetAddressOf());
	depth->Release();
	view->Release();
	con->Release();
}