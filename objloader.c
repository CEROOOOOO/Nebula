#define STB_IMAGE_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION  // keep if you want it later

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cgltf.h>
#include "buffer.h"
#include "stb_image.h"
#include "vertex.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>

uint32_t vertexcount = 0;
uint32_t indexcount = 0;
Vertex *vertices = NULL;
uint32_t *indices = NULL;
unsigned char *pixels = NULL;
int texWidth = 0, texHeight = 0, texChannels = 0;

bool assimp()
{
    const struct aiScene *scene = aiImportFile(
        "/home/prashanth/Desktop/nebula/assets/moon(1).glb",
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices
    );

    if (!scene || scene->mNumMeshes == 0) {
        printf("Assimp error: %s\n", aiGetErrorString());
        return false;
    }

    printf("Meshes: %u\n", scene->mNumMeshes);

    vertexcount = 0;
    indexcount = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const struct aiMesh *mesh = scene->mMeshes[i];
        vertexcount += mesh->mNumVertices;
        indexcount += mesh->mNumFaces * 3;
    }

    vertices = malloc(sizeof(Vertex) * vertexcount);
    indices  = malloc(sizeof(uint32_t) * indexcount);
    if (!vertices || !indices) {
        printf("Memory allocation failed\n");
        return false;
    }

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const struct aiMesh *mesh = scene->mMeshes[m];

        // Vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            uint32_t idx = vertexOffset + i;

            vertices[idx].position[0] = mesh->mVertices[i].x;
            vertices[idx].position[1] = mesh->mVertices[i].y;
            vertices[idx].position[2] = mesh->mVertices[i].z;

            if (mesh->mTextureCoords[0]) {
                vertices[idx].texcord[0] = mesh->mTextureCoords[0][i].x;
                vertices[idx].texcord[1] = mesh->mTextureCoords[0][i].y;
            } else {
                vertices[idx].texcord[0] = 0.0f;
                vertices[idx].texcord[1] = 0.0f;
            }
        }

        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            const struct aiFace *face = &mesh->mFaces[f];
            if (face->mNumIndices != 3) continue; // should be triangulated

            indices[indexOffset++] = vertexOffset + face->mIndices[0];
            indices[indexOffset++] = vertexOffset + face->mIndices[1];
            indices[indexOffset++] = vertexOffset + face->mIndices[2];
        }

        vertexOffset += mesh->mNumVertices;
    }

    if (scene->mNumTextures > 0) {
        const struct aiTexture *tex = scene->mTextures[0];

        if (tex->mHeight == 0) {
            // Compressed format (PNG, JPG, etc.) — most common for .glb
            pixels = stbi_load_from_memory(
                (unsigned char*)tex->pcData,
                tex->mWidth,           // size in bytes
                &texWidth, &texHeight, &texChannels,
                STBI_rgb_alpha
            );
        }

        if (!pixels) {
            printf("stb_image failed: %s\n", stbi_failure_reason());
        } else {
            printf("Loaded texture: %dx%d channels=%d\n", texWidth, texHeight, texChannels);
        }
    } else {
        printf("No embedded textures found.\n");
    }

    aiReleaseImport(scene);
    return true;
}

void obj_cleanup()
{
    free(vertices);   vertices = NULL;
    free(indices);    indices = NULL;
    if (pixels) {
        stbi_image_free(pixels);
        pixels = NULL;
    }
    vertexcount = indexcount = 0;
    texWidth = texHeight = texChannels = 0;
}