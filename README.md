# VoxWorld
A Minecraft Clone Built using C++ and OpenGL

## What's Included
* Infinite multi threaded chunk generation
* Breakable blocks
* Player physics (jumping, breaking blocks, sprinting)

I'm using minecrafts assets right now. I'll be switching to my own once I get further into the project.



https://github.com/user-attachments/assets/625cc971-2950-4cd2-a337-d86eac7dd373



https://github.com/user-attachments/assets/b9c1d8b9-3c5b-4ab3-b6a7-041fbac6231b



![voxwrld_a0 0 1](https://github.com/user-attachments/assets/22810576-e423-4894-93fe-85bbcda90403)
<img width="1444" height="825" alt="image" src="https://github.com/user-attachments/assets/d143de30-6a65-4678-8926-feee08e81c77" />

Book I used to help me get through this project:
- [OpenGL Docs](https://docs.gl/)

## Libraries Used
- glad
- glfw
- glm
- Perlin noise

## Build
Note: Clone recursivly to get the nested libraries.

#### Linux
```
cd path/of/voxwrld
mkdir build && cd build && cmake .. && cmake --build .
```


