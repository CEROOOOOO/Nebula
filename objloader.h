#pragma once
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <stdbool.h>
#include "vertex.h"
extern unsigned char *pixels;
extern int texWidth;
extern int texHeight;
extern int texChannels; // no need to pass my original details stb image will fill it automatically
extern Vertex *vertices;
extern uint32_t* indices;
extern uint32_t indexcount;
extern uint32_t vertexcount;
bool assimp();
void obj_cleanup();