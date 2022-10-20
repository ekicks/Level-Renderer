#include "Model.h"

void Model::SetData(Model& model, GW::GRAPHICS::GDirectX11Surface d3d, ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ID3D11DepthStencilView* depth) {

	// setup the pipeline
	ID3D11RenderTargetView* const views[] = { view };
	con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);
	const UINT strides[] = { sizeof(H2B::VERTEX) };
	const UINT offsets[] = { 0 };
	ID3D11Buffer* const buffs[] = { model.vertexBuffer.Get() };
	con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	con->IASetIndexBuffer(model.indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	con->VSSetConstantBuffers(0, 1, model.constantBuffer.GetAddressOf());
	con->PSSetConstantBuffers(0, 1, model.constantBuffer.GetAddressOf());
	con->UpdateSubresource(model.constantBuffer.Get(), 0, nullptr, &model.modelWoldStructs, 0, 0);
}


void Model::CreateBuffer(ID3D11Device* creator, GW::GRAPHICS::GDirectX11Surface _d3d)
{
	CD3D11_BUFFER_DESC cDesc(sizeof(Model::ConstWorld), D3D11_BIND_CONSTANT_BUFFER);
	creator->CreateBuffer(&cDesc, nullptr, constantBuffer.GetAddressOf());

	D3D11_SUBRESOURCE_DATA bData = { parser.vertices.data(), 0, 0 };
	CD3D11_BUFFER_DESC bDesc(parser.vertices.size() * sizeof(H2B::VERTEX), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());

	D3D11_SUBRESOURCE_DATA iData = { parser.indices.data(), 0, 0 };
	CD3D11_BUFFER_DESC iDesc(parser.indices.size() * sizeof(unsigned), D3D11_BIND_INDEX_BUFFER);
	creator->CreateBuffer(&iDesc, &iData, indexBuffer.GetAddressOf());

}

void Model::LoadModel(Model* model, GW::GRAPHICS::GDirectX11Surface* d3d, ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ID3D11DepthStencilView* depth,
	Microsoft::WRL::ComPtr<ID3D11Buffer> colorBuffer, ColorBuff* colorBuff) {

	model->SetData(*model, *d3d, con, view, depth);
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (int i = 0; i < model->parser.materialCount; i++)
	{
		colorBuff->outputColor = model->parser.materials[i].attrib;
		con->PSSetConstantBuffers(2, 1, colorBuffer.GetAddressOf());
		con->UpdateSubresource(colorBuffer.Get(), 0, nullptr, colorBuff, 0, 0);
		con->DrawIndexed(model->parser.meshes[i].drawInfo.indexCount, model->parser.meshes[i].drawInfo.indexOffset, 0);
	}

}
Model::Model()
{
}

Model::~Model()
{
}