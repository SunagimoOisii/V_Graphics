#include <vector>

#include "Mesh.h"

/// <summary>
/// ����Mesh�N���X���Ǘ�����N���X
/// </summary>
class Model
{
public:
    const std::vector<Mesh>& GetMeshes() const { return meshes; }
    void AddMesh(const Mesh& mesh) { meshes.push_back(mesh); }

private:
    std::vector<Mesh> meshes;
};