# WaveSimGitHub
Please have a look at the dev diary for this project for my thought process while working on this project here: [DEV DIARY](https://arun95pillai7.wixsite.com/arpwarp/wave-simulation-project-updates)

 Wave simulation using a DirectX framework taken from Dr. Paul Varcholik's open source project ( https://bitbucket.org/pvarcholik/real-time-3d-rendering-with-directx-and-hlsl/src/master/ ). Extended on the framework to simulate waves using heightfields. The framework uses a master material supporting all shader stages. I added functionality for Texture1D, Texture1D Array, and Texture2D to the framework, as well as creating the materials and shaders associated with the simulation.

 First implementation of the simulation was done by calculating the height values for each vertex and using this to update the z-values for each vertex.
 In the second iteration, the z value was split off from the x and y coordinates of the vertex information. After calculating height values, this information was written to a 1D texture to be passed to the GPU. In the vertex shader, a vertex index is used to sample the texture and offset the vertex by the appropriate z value. Additionally, an algorithm to create an index buffer from the given grid size was written, and used to generate a wireframe mesh for the grid.

 Ongoing- using compute shaders to do the physics calculations, and write to a texture locally on the GPU, to be then used by the previously implemented texture sampling method to update the vertices. This eliminates transfore of data from CPU to GPU for each draw call, and the entire simulation can be carried out with just 2 GPU draw calls. 
