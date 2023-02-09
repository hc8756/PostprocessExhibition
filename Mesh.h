#pragma once

#include "DXCore.h"
#include "Vertex.h"
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <fstream>
#include <vector>

	class Mesh {
	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
		int GetIndexCount();
		void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
		Mesh(Vertex* vertexArray, int vertexNum, int* indexArray, int indexNum, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
		Mesh(const char* file, Microsoft::WRL::ComPtr<ID3D11Device> device);
		~Mesh();
		void CreateBuffers(Vertex* vertexArray, int vertexNum, int* indexArray, int indexNum, Microsoft::WRL::ComPtr<ID3D11Device> device);
		void CalculateTangents(Vertex* verts, int numVerts, int* indices, int numIndices);
	private:
		// Buffers to hold actual geometry data
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> myContext;
		int index;

	};
