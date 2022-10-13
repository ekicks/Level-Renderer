#include "Model.h"

class Level
{

public:
	std::string name;
	std::vector<Model> meshes;
	bool LoadLevel(const char* filepath);
	void DrawModel(Model model, GW::GRAPHICS::GDirectX11Surface d3d);
};