#include "SceneLayer/Mesh.h"

Mesh::Mesh(VkDevice d, 
	const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: dev(d)
	, vertices(vertices)
	, indices(indices)
	, vertexBuf(VK_NULL_HANDLE)
	, indexBuf(VK_NULL_HANDLE)
{
}

Mesh::~Mesh()
{
	//���_,�C���f�b�N�X�o�b�t�@�̔j��
	if (vertexBuf != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(dev, vertexBuf, nullptr);
		vertexBuf = VK_NULL_HANDLE;
	}
	if (indexBuf != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(dev, indexBuf, nullptr);
		indexBuf = VK_NULL_HANDLE;
	}
}