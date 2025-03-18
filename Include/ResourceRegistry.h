#pragma once

#include <unordered_map>
#include <string>
#include <any>
#include <stdexcept>
#include <iostream>

/// <summary>
/// リソースを登録,取得できるクラス<para></para>
/// 特定リソースのためにクラス変数やstd::function型変数でゲッターを注入する必要を
/// なくすために作成された<para></para>
/// リソース生成側が登録したリソースを、利用側が名前キーを用いて取得する
/// </summary>
class ResourceRegistry
{
public:
    template<typename T>
    void RegisterResource(const char* name, T resource)
    {
        resources[name] = std::make_any<T>(std::move(resource));

        //ログ出力
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