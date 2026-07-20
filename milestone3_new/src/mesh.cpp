#include "mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <stdexcept>
#include <iostream>


template<typename T> static void hash_combine(size_t& seed, const T& v){
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct VtxKey{
    int vi, ni, ti;

    bool operator==(const VtxKey& o) const noexcept{
        return vi == o.vi && ni == o.ni && ti == o.ti;
    }
};

struct VtxKeyHash{
    size_t operator()(const VtxKey& k) const noexcept{
        size_t h = 0;
        hash_combine(h, k.vi);
        hash_combine(h, k.ni);
        hash_combine(h, k.ti);
        return h;
    }
};


Mesh loadModel(const std::string& path){
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        throw std::runtime_error(warn + err);

    if(!warn.empty())
        std::cout << "[tinyobj warn] " << warn << '\n';

    Mesh mesh;
    std::unordered_map<VtxKey, uint32_t, VtxKeyHash> uniqueVertices;

    for(const auto& shape : shapes){
        for(const auto& index : shape.mesh.indices){
            Vertex v{};

            v.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if(index.normal_index >= 0){
                v.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if(index.texcoord_index >= 0){
                v.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]  // flip V for Vulkan
                };
            }

            // try_emplace performs a single map lookup:
            //   - on insertion  → stores the new vertex index and pushes the vertex
            //   - on collision  → leaves the existing entry untouched
            const VtxKey key{index.vertex_index, index.normal_index, index.texcoord_index};
            auto[it, inserted] = uniqueVertices.try_emplace(key, static_cast<uint32_t>(mesh.vertices.size()));

            if(inserted){
                mesh.vertices.push_back(v);
            }

            mesh.indices.push_back(it->second);
        }
    }

    return mesh;
}