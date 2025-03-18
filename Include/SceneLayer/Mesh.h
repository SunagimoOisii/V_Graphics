#include <vector>

#include "Vertex.h"

/// <summary>
/// 1メッシュ分の頂点,インデックスバッファの管理を行うクラス
/// </summary>
class Mesh
{
public:
    Mesh(VkDevice d,
        const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    VkBuffer GetVertexBuffer() const { return vertexBuf; }
    VkBuffer GetIndexBuffer()  const { return indexBuf; }
    uint32_t GetIndexCount()   const { return static_cast<uint32_t>(indices.size()); }

    void SetVertexBuffer(VkBuffer buf) { vertexBuf = buf; }
    void SetIndexBuffer (VkBuffer buf) { indexBuf = buf; }

private:
//***********************************************************
// 変数↓
//***********************************************************

    VkDevice dev;

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    VkBuffer vertexBuf;
    VkBuffer indexBuf;
};