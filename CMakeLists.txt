cmake_minimum_required(VERSION 3.23)#so see cmake min version idk why
project(nebula C)#just name it anything i never saW IT anywhere
find_package(Vulkan REQUIRED)#vulkan required cause its in file na 
find_package(glfw3 REQUIRED)#its also in file these are in usr space means i downloaded from terminal 
find_package(assimp REQUIRED)
add_executable(nebula)#executable name cglm is downloaded rom github so extra care means give address 
target_include_directories(nebula PRIVATE /home/prashanth/cglm/cglm-0.9.6/include
/home/prashanth/cgltf-1.15
)
target_sources(nebula
  PRIVATE
  instance.c
  device.c
  present.c
  buffer.c
  cmdbuffer.c
  renderpass.c
  pipeline.c
  descriptor.c
  objloader.c
  main.c
    )#like the source files here my all .c vulkan will come here now wooh  
target_link_libraries(nebula
Vulkan::Vulkan
glfw
assimp
m
)#linking lib
#linking asan 
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    if(MSVC)
        # For Microsoft Visual C++
        target_compile_options(nebula PRIVATE /fsanitize=address)
    else()
        # For GCC and Clang
        target_compile_options(nebula PRIVATE -g -fsanitize=address)
        target_link_options(nebula PRIVATE -fsanitize=address)
    endif()
endif()
