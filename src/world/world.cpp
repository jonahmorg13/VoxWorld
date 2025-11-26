#include <vector>
#include <iostream>
#include <thread>
#include <atomic>

#include "world/world.h"
#include "world/chunkData.h"
#include "world/chunkMesh.h"
#include "world/chunkPos.h"
#include "block.h"

#include <chrono>

void World::init()
{
    focusMesh.init();
    // TODO: fix this call, 0, 0? what if theyre not at spawn. dumbass...
    generateChunkDataFromPos({0, 0}, true);
    intialDataGenerated = true;
}

BLOCK World::getBlockData(glm::ivec3 blockPos)
{
    int chunk_x = blockPos.x / CHUNK_SIZE;
    int chunk_z = blockPos.z / CHUNK_SIZE;

    int block_x = blockPos.x % CHUNK_SIZE;
    int block_y = blockPos.y;
    int block_z = blockPos.z % CHUNK_SIZE;

    if (block_x < 0)
    {
        block_x += CHUNK_SIZE;
        chunk_x -= 1;
    }

    if (block_y < 0)
    {
        block_y += CHUNK_SIZE;
    }

    if (block_z < 0)
    {
        block_z += CHUNK_SIZE;
        chunk_z -= 1;
    }

    ChunkPos chunkPos = {chunk_x,
                         chunk_z};

    if (!chunkDataExists(chunkPos))
    {
        // TODO: this is bullshit, fix this later
        return BLOCK::AIR_BLOCK;
    }
    std::vector<char> data = getChunkDataIfExists(chunkPos);
    if (data.size() > 0)
    {
        BLOCK block = (BLOCK)data[block_x + (block_y * CHUNK_SIZE) + (block_z * CHUNK_SIZE * CHUNK_HEIGHT)];
        // std::cout << "Found block: " << block << std::endl
        //           << "at chunk: (" << chunkPos.x << ", " << chunkPos.z << ")" << std::endl
        //           << "at pos: (" << block_x << ", " << block_y << ", " << block_z << ")" << std::endl;
        return block;
    }

    return BLOCK::AIR_BLOCK;
}

void World::removeBlock(glm::ivec3 blockPos)
{
    int chunk_x = blockPos.x / CHUNK_SIZE;
    int chunk_z = blockPos.z / CHUNK_SIZE;

    int block_x = blockPos.x % CHUNK_SIZE;
    int block_y = blockPos.y;
    int block_z = blockPos.z % CHUNK_SIZE;

    if (block_x < 0)
    {
        block_x += CHUNK_SIZE;
        chunk_x -= 1;
    }

    if (block_y < 0)
    {
        block_y += CHUNK_SIZE;
    }

    if (block_z < 0)
    {
        block_z += CHUNK_SIZE;
        chunk_z -= 1;
    }

    ChunkPos chunkPos = {chunk_x,
                         chunk_z};

    std::vector<char> &data = getChunkDataIfExists(chunkPos);
    if (data.size() > 0)
    {
        data[block_x + (block_y * CHUNK_SIZE) + (block_z * CHUNK_SIZE * CHUNK_HEIGHT)] = BLOCK::AIR_BLOCK;
        auto testData = chunkDataMap[chunkPos];
        BLOCK testBlock = (BLOCK)testData[block_x + (block_y * CHUNK_SIZE) + (block_z * CHUNK_SIZE * CHUNK_HEIGHT)];

        std::unique_lock<std::mutex> queue_lock(mesh_queue_mtx);
        chunksToMeshQueue.push_front(chunkPos);
        if (block_z <= 0)
        {
            chunksToMeshQueue.push_front({chunkPos.x, chunkPos.z - 1});
        }
        if (block_z >= CHUNK_SIZE - 1)
        {
            chunksToMeshQueue.push_front({chunkPos.x, chunkPos.z + 1});
        }
        if (block_x <= 0)
        {
            chunksToMeshQueue.push_front({chunkPos.x - 1, chunkPos.z});
        }
        if (block_x >= CHUNK_SIZE - 1)
        {
            chunksToMeshQueue.push_front({chunkPos.x + 1, chunkPos.z});
        }
    }
}

//TODO refactor this code to for chunk mesh queue and chunk pos calculating
void World::createBlock(glm::ivec3 blockPos, BLOCK block) {
    int chunk_x = blockPos.x / CHUNK_SIZE;
    int chunk_z = blockPos.z / CHUNK_SIZE;

    int block_x = blockPos.x % CHUNK_SIZE;
    int block_y = blockPos.y;
    int block_z = blockPos.z % CHUNK_SIZE;

    if (block_x < 0)
    {
        block_x += CHUNK_SIZE;
        chunk_x -= 1;
    }

    if (block_y < 0)
    {
        block_y += CHUNK_SIZE;
    }

    if (block_z < 0)
    {
        block_z += CHUNK_SIZE;
        chunk_z -= 1;
    }

    ChunkPos chunkPos = {chunk_x,
                         chunk_z};

    std::vector<char> &data = getChunkDataIfExists(chunkPos);
    if (data.size() > 0)
    {
        data[block_x + (block_y * CHUNK_SIZE) + (block_z * CHUNK_SIZE * CHUNK_HEIGHT)] = block;

        std::unique_lock<std::mutex> queue_lock(mesh_queue_mtx);
        chunksToMeshQueue.push_front(chunkPos);
        if (block_z <= 0)
        {
            chunksToMeshQueue.push_front({chunkPos.x, chunkPos.z - 1});
        }
        if (block_z >= CHUNK_SIZE - 1)
        {
            chunksToMeshQueue.push_front({chunkPos.x, chunkPos.z + 1});
        }
        if (block_x <= 0)
        {
            chunksToMeshQueue.push_front({chunkPos.x - 1, chunkPos.z});
        }
        if (block_x >= CHUNK_SIZE - 1)
        {
            chunksToMeshQueue.push_front({chunkPos.x + 1, chunkPos.z});
        }
    }
}

void World::startWorldGeneration()
{
    // Locking current position initially to avoid races
    {
        // Enqueue data generation task
        dataThreadPool.enqueue([this]
                               {
            ChunkPos newPos;
            while (true)
            {
                // Lock to safely read world position
                {
                    std::unique_lock<std::mutex> lock(pos_mtx);
                    newPos = worldCurrPos;
                }

                //if (newPos == currentPos) continue;
                generateChunkDataFromPos(newPos, false);

                // Add a break condition or sleep for efficiency
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
            } });

        // Enqueue mesh generation task
        threadPool.enqueue([this]
                           {
            ChunkPos newPos;

            while (true)
            {
                // Lock to safely read world position
                {
                    std::unique_lock<std::mutex> lock(pos_mtx);
                    newPos = worldCurrPos;
                }

                bool hasChunksToMesh = false;

                // Lock to safely check the chunk queue
                {
                    std::unique_lock<std::mutex> lock(mesh_queue_mtx);
                    hasChunksToMesh = !chunksToMeshQueue.empty();
                }

                if (!hasChunksToMesh)
                {
                    addChunksToMeshQueue(newPos);
                }

                // Add a break condition or sleep for efficiency
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } });

        threadPool.enqueue([this]
                           { while(true) {
                            std::unique_lock<std::mutex> lock(mesh_queue_mtx);
                                if (!chunksToMeshQueue.empty())
                                {
                                    lock.unlock();
                                    generateNextMesh();
                                }
                                 } });

        dataThreadPool.enqueue([this]
                               {
            ChunkPos newPos;
            while (true)
            {
                // Lock to safely read world position
                {
                    std::unique_lock<std::mutex> lock(pos_mtx);
                    newPos = worldCurrPos;
                }

                //if (newPos == currentPos) continue;
                removeUnneededChunkData(newPos);

                // Add a break condition or sleep for efficiency
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } });

        threadPool.enqueue([this]
                           {
            ChunkPos newPos;
            while (true)
            {
                // Lock to safely read world position
                {
                    std::unique_lock<std::mutex> lock(pos_mtx);
                    newPos = worldCurrPos;
                }

                //if (newPos == currentPos) continue;
                removeUnneededChunkMeshes(newPos);

                // Add a break condition or sleep for efficiency
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } });
    }
}

void World::render()
{
    renderChunkMeshes();
    focusMesh.draw();
    focusMesh.reset();
}

void World::updateFocusBlock(glm::ivec3 &pos, char &face)
{
    unsigned int indiceOffset = 0;

    std::vector<UVcoords> textureCoords = blockTextureCoords[BLOCK::FOCUS];
    UVcoords bottomTexCoords = textureCoords[0];
    UVcoords sideTexCoords = textureCoords[1];
    UVcoords topTexCoords = textureCoords[2];

    glm::vec3 blockPos = glm::vec3(pos.x, pos.y, pos.z);

    BlockRenderInfo renderInfo = {
        BLOCK::FOCUS,
        face,
        blockPos,
        focusMesh.vertices,
        focusMesh.indices,
        indiceOffset};

    blockRenderFunctions[BLOCK::FOCUS](renderInfo);
}