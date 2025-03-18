#pragma once

#include <unordered_map>
#include <string>
#include <any>
#include <stdexcept>
#include <iostream>

/// <summary>
/// ���\�[�X��o�^,�擾�ł���N���X<para></para>
/// ���胊�\�[�X�̂��߂ɃN���X�ϐ���std::function�^�ϐ��ŃQ�b�^�[�𒍓�����K�v��
/// �Ȃ������߂ɍ쐬���ꂽ<para></para>
/// ���\�[�X���������o�^�������\�[�X���A���p�������O�L�[��p���Ď擾����
/// </summary>
class ResourceRegistry
{
public:
    template<typename T>
    void RegisterResource(const char* name, T resource)
    {
        resources[name] = std::make_any<T>(std::move(resource));

        //���O�o��
        std::cout << "ResourceRegistry: Registered resource: "
            << name
            << " (Type: " << typeid(T).name() << ")"
            << std::endl;
    }

    template<typename T>
    T FindResource(const char* name) const
    {
        auto it = resources.find(name);
        if (it != resources.end())
        {
            return std::any_cast<const T&>(it->second);
        }
        throw std::runtime_error(std::string("Resource not found: ") + name);
    }

private:
    std::unordered_map<const char*, std::any> resources;
};