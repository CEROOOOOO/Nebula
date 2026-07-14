cmake_minimum_required(VERSION 3.23)#so see cmake min version idk why
project(nebula C)#just name it anything i never saW IT anywhere
find_package(Vulkan REQUIRED)#vulkan required cause its in file na 
find_package(glfw3 REQUIRED)#its also in file these are in usr space means i downloaded from terminal 
add_executable(nebula)#executable name cglm is downloaded rom github so extra care means give address 
target_include_directories(nebula PRIVATE /home/prashanth/cglm/cglm-0.9.6/include)
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
  main.c
    )#like the source files here my all .c vulkan will come here now wooh  
target_link_libraries(nebula
Vulkan::Vulkan
glfw
m
)#linking lib