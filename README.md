This is a fork of https://projects.blender.org/blender/blender.git
# Final Project - Introduction to Computer Graphics
In this course, we learned about how computers render 3D objects. When performing ray-tracing, one must model the function by which light bounces off an object, through something called a bidirectional reflectance distribution function (BRDF). For my final project, I wanted to try implementing some of the BRDF functions we learned, specifically the Phong and Blinn-Phong BRDFs. I did this by modifying the open-source blender software to add custom models. Note that this is NOT a supported feature of blender and therefore required extensive reverse engineering and modifications to the blender source code, contained in this repository.

## Modifying Blender

I started from the blender 3.5.1 source code (tag v3.5.1 of https://projects.blender.org/blender/blender.git) and made modifications to add my BRDFs. The respository contains the **entire** blender source code, as copied. The files that I modified are listed below, and for most files, this amounts to adding another entry in a list or adding a new type to parallel the existing ones. To see the modifications, it may be easier to go into the commit diff view. As blender is written in C++, nearly everything I did is in C++, with the exception of the BRDF itself, which is written as a shader (in a subset of C that blender uses to cross-compile its shaders across CUDA, OpenGL, and Metal).

```
.
└── blender/
    ├── build_files/
    │   ├── utils/
    │   │   └── make_update.py
    │   └── windows/
    │       └── check_libraries.cmd
    ├── intern/
    │   └── cycles/
    │       ├── blender/
    │       │   └── shader.cpp
    │       ├── kernel/
    │       │   ├── closure/
    │       │   │   ├── bsdf.h
    │       │   │   └── bsdf_bradley.h
    │       │   ├── CMakeLists.txt
    │       │   └── svm/
    │       │       ├── closure.h
    │       │       └── types.h
    │       └── scene/
    │           ├── shader_nodes.cpp
    │           └── shader_nodes.h
    ├── scripts/
    │   └── startup/
    │       └── nodeitems_builtins.py
    └── source/
        └── blender/
            ├── blenkernel/
            │   └── BKE_node.h
            ├── editors/
            │   └── space_node/
            │       └── drawnode.cc
            ├── makesdna/
            │   └── DNA_node_types.h
            ├── makesrna/
            │   └── intern/
            │       └── rna_nodetree.c
            └── nodes/
                ├── NOD_static_types.h
                └── shader/
                    ├── CMakeLists.txt
                    ├── nodes/
                    │   └── node_shader_bsdf_bradley.cc
                    ├── node_shader_register.cc
                    ├── node_shader_register.hh
                    └── node_shader_tree.cc
```
### BSDF_bradley.h
This is where the main source code is. Specifically, it contains the functions that directly compute the brdf: ``bradley_brdf_eval`` and ``bradley_brdf_sample``. As the names imply, ``eval`` evaluates the brdf for a specific imcoming and outgoing light ray (``wi`` is the vector pointing towards the camera, where each path originates from, and ``wo`` is the vector pointing towards the light source). It returns a vector (Spectrum) of the brdf for R,G,B. I believe cycles uses this when it knows where the light is and wants to test the brdf at a specific angle (i.e. towards the light source). ``sample`` just takes an incoming light ray, as well as two random floats ``u`` and ``v`` ranging from 0 to 1 (I believe cycles is actually smart about these and they are not entirely random but that is beyond my understanding). It then decides on an output vector, as well as the ``pdf`` of that vector (which I assume gets used for Monte Carlo approximation down the line) and the  brdf evaluated for the output vector. In my implementation, I sample uniformly along the hemisphere surrounding the normal vector (``N`` which is the computed, potentially smoothed normal, rather than ``Ng`` which is the true geometric normal). In other words, I do not employ importance sampling, resulting in a potentially noisier image.

### Other files
All other files are to enable the custom shader to appear and function in the node editor. Files in ``source/blender`` deal with adding the node to the UI with the parameters and drop-down menu I wanted. The file in ``intern/cycles/blender`` translates the ENUM corresponding to the drop-down menu in my node into an ENUM cycles can understand. The files in ``intern/cycles/scene`` run on the CPU and translate the node-graph in the blender-gui into an encoded format to be rendered by cycles. I simply added my node there. Then, the files ``intern/cycles/kernel`` run on the GPU/CPU in the cycles virtual machine. ``intern/cycles/kernel/svm/closure.h`` decodes the encoded graph (wherein I once again had to add my node). If you want to find where in each of those files I made changes, you should be able just CMD-F "bradley" as most of the structs follow the same naming convention (which has my name in it :)).

### Limitations
I did not test whether my node works with OSL and I have a feeling it does not (since I skipped that part in ``intern/cycles/scene/shader_nodes.cpp``) but you are welcome to try/fix it. Additionally, if you select viewport rendering (the second to last option on the top right in blender), anything using this shader will render as a black box since I did not implement the corresponding OpenGL.

### Building Blender
To build blender with the custom node, clone this repository follow the instructions at [Building Blender - Blender Developer Wiki](https://wiki.blender.org/wiki/Building_Blender) to build blender. If you would like to build with GPU support, there are additional steps.
