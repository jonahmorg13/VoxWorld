#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include "rendering.h"
#include "texture.h"

enum BLOCK
{
    AIR_BLOCK = 0,
    GRASS_BLOCK = 1,
    DIRT_BLOCK = 2,
    STONE_BLOCK = 3,
    FOCUS = 4,
    BEDROCK_BLOCK = 5,
    SNOW_BLOCK = 6,
    WATER_BLOCK = 7,
    SAND_BLOCK = 8,
    OAK_WOOD = 9,
    OAK_LEAVES = 10
};

static std::unordered_map<BLOCK, std::string> blockNames = {
    {BLOCK::AIR_BLOCK, "Air"},
    {BLOCK::GRASS_BLOCK, "Grass"},
    {BLOCK::DIRT_BLOCK, "Dirt"},
    {BLOCK::STONE_BLOCK, "Stone"},
    {BLOCK::FOCUS, "Focus"},
    {BLOCK::BEDROCK_BLOCK, "Bedrock"},
    {BLOCK::SNOW_BLOCK, "Snow"},
    {BLOCK::WATER_BLOCK, "Water"},
    {BLOCK::SAND_BLOCK, "Sand"},
    {BLOCK::OAK_WOOD, "Oak Wood"},
    {BLOCK::OAK_LEAVES, "Oak Leaves"}
};

typedef struct
{
    int x, y, z;
    BLOCK block;
} BlockWithPos;

typedef struct
{
    BLOCK block;
    char cover;
    glm::vec3 blockPos;
    std::vector<Vertex> &chunkVertices;
    std::vector<unsigned int> &chunkIndices;
    unsigned int &indiceOffset;
} BlockRenderInfo;

typedef struct
{
    BLOCK block;
    char cover;
    glm::vec3 blockPos;
    std::vector<Vertex> &chunkVertices;
    std::vector<unsigned int> &chunkIndices;
    unsigned int &indiceOffset;
    bool liquidOnTop;
} LiquidRenderInfo;

void renderRegularBlock(BlockRenderInfo &renderInfo);
void renderLiquidBlock(LiquidRenderInfo &renderInfo);
void renderAirBlock(BlockRenderInfo &renderInfo);

static std::unordered_map<BLOCK, std::vector<UVcoords>> blockTextureCoords = {
    {BLOCK::AIR_BLOCK, {}},
    {BLOCK::GRASS_BLOCK, {getTextureCoordsFromAtlas(0, 0), getTextureCoordsFromAtlas(0, 1), getTextureCoordsFromAtlas(0, 2)}},
    {BLOCK::DIRT_BLOCK, {getTextureCoordsFromAtlas(0, 0), getTextureCoordsFromAtlas(0, 0), getTextureCoordsFromAtlas(0, 0)}},
    {BLOCK::STONE_BLOCK, {getTextureCoordsFromAtlas(0, 3), getTextureCoordsFromAtlas(0, 3), getTextureCoordsFromAtlas(0, 3)}},
    {BLOCK::FOCUS, {getTextureCoordsFromAtlas(0, 4), getTextureCoordsFromAtlas(0, 4), getTextureCoordsFromAtlas(0, 4)}},
    {BLOCK::BEDROCK_BLOCK, {getTextureCoordsFromAtlas(0, 5), getTextureCoordsFromAtlas(0, 5), getTextureCoordsFromAtlas(0, 5)}},
    {BLOCK::SNOW_BLOCK, {getTextureCoordsFromAtlas(0, 6), getTextureCoordsFromAtlas(0, 6), getTextureCoordsFromAtlas(0, 6)}},
    {BLOCK::WATER_BLOCK, {getTextureCoordsFromAtlas(0, 7)}},
    {BLOCK::SAND_BLOCK, {getTextureCoordsFromAtlas(0, 8), getTextureCoordsFromAtlas(0, 8), getTextureCoordsFromAtlas(0, 8)}},
    {BLOCK::OAK_WOOD, {getTextureCoordsFromAtlas(0, 11), getTextureCoordsFromAtlas(0, 9), getTextureCoordsFromAtlas(0, 11)}},
    {BLOCK::OAK_LEAVES, {getTextureCoordsFromAtlas(0, 10), getTextureCoordsFromAtlas(0, 10), getTextureCoordsFromAtlas(0, 10)}},
};

static std::unordered_map<BLOCK, void (*)(BlockRenderInfo &renderInfo)> blockRenderFunctions = {
    {BLOCK::AIR_BLOCK, renderAirBlock},
    {BLOCK::GRASS_BLOCK, renderRegularBlock},
    {BLOCK::DIRT_BLOCK, renderRegularBlock},
    {BLOCK::STONE_BLOCK, renderRegularBlock},
    {BLOCK::FOCUS, renderRegularBlock},
    {BLOCK::BEDROCK_BLOCK, renderRegularBlock},
    {BLOCK::SNOW_BLOCK, renderRegularBlock},
    {BLOCK::SAND_BLOCK, renderRegularBlock},
    {BLOCK::OAK_WOOD, renderRegularBlock},
    {BLOCK::OAK_LEAVES, renderRegularBlock},
};

static std::unordered_map<BLOCK, void (*)(LiquidRenderInfo &renderInfo)> liquidRenderFunctions = {
    {BLOCK::WATER_BLOCK, renderLiquidBlock},
};

static bool isLiquid(BLOCK block)
{
    return block == BLOCK::WATER_BLOCK;
}
