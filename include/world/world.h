#pragma once

#include <deque>
#include <mutex>

#include "world/chunkData.h"
#include "world/chunkMesh.h"
#include "block.h"
#include "world/mesh.h"
#include "threading.h"

class World
{
public:
    World() : threadPool(3), dataThreadPool(3)
    {
        focusMesh.setDepthTest(false);
        intialDataGenerated = false;
    }

    void init();
    void startWorldGeneration();

    void render();
    BLOCK getBlockData(glm::ivec3 blockPos);
    void removeBlock(glm::ivec3 blockPos);
    void createBlock(glm::ivec3 blockPos, BLOCK block);
    void updateFocusBlock(glm::ivec3 &pos, char &face);
    bool intialDataGenerated;
    ChunkPos worldCurrPos;
    std::mutex pos_mtx;

private:
    ThreadPool threadPool;
    ThreadPool dataThreadPool;
    int chunkGenerationTries = 0;

    std::mutex data_mtx;
    ChunkDataMap chunkDataMap;

    std::mutex mesh_mtx;
    ChunkMeshMap chunkMeshMap;

    std::mutex mesh_queue_mtx;
    std::deque<ChunkPos> chunksToMeshQueue;

    std::mutex data_queue_mtx;
    std::deque<ChunkPos> chunkDataQueue;

    std::mutex struct_mtx;
    StructQueue structQueue;

    std::mutex player_mtx;

    Mesh focusMesh;

    bool posIsInQueue(std::deque<ChunkPos> &queue, ChunkPos &pos);

    // chunk data
    void addChunksToDataQueue(ChunkPos &chunkPos);
    void generateNextData();

    void generateChunkDataFromPos(ChunkPos pos, bool initial);
    void generateChunkData(ChunkPos pos);
    std::vector<char> &getChunkDataIfExists(ChunkPos pos);
    bool chunkDataExists(ChunkPos chunkPos);

    void removeChunkDataFromMap(ChunkPos pos);
    void removeUnneededChunkData(ChunkPos pos);

    // chunk mesh
    void initializeOpaqueChunk(ChunkMesh &chunkMesh);
    void initializeTransparentChunk(ChunkMesh &chunkMesh);
    void bindChunkOpaque(ChunkMesh &chunkMesh);
    void bindChunkTransparent(ChunkMesh &chunkMesh);
    void unbindChunk(ChunkMesh &chunkMesh);
    void addChunksToMeshQueue(ChunkPos pos);

    void renderChunkMeshes();
    void generateNextMesh();

    bool chunkMeshExists(ChunkPos pos);
    void removeChunkFromMap(ChunkPos pos);
    void removeUnneededChunkMeshes(ChunkPos pos);
    ChunkMesh *getChunkFromMap(ChunkPos pos);

    // structures
    void addStructureBlockToWorld(ChunkPos &pos, BlockWithPos &blockWithPos, std::vector<char> &data);
    void generateStructures(std::vector<char> &data, ChunkPos pos);
};