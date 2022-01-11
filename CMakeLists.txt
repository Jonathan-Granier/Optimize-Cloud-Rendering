cmake_minimum_required(VERSION 3.0.0)
project(CloudRendering)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/Olympus/cmake")

add_executable(CloudRendering)

# Using C++17
target_compile_features(CloudRendering PRIVATE cxx_std_17)

############################
# Olympus - Compiler flags #
############################ 

include(CompilerFlags) 
add_compiler_flags(CloudRendering PRIVATE)
 
   
#############################
# Olympus - Finding Vulkan #
############################# 
   
if (NOT VULKAN_SDK_ROOT)
    # Vulkan SDK path not set, trying to find it automatically
    find_package(Vulkan) 
endif ()
  
if (Vulkan_FOUND)
    if (NOT GLSLC_EXECUTABLE)
        set(GLSLC_EXECUTABLE glslc) # glslc is assumed to be available from the path... Kind of a long shot under Windows
    endif () 

    if (GLSLC_EXECUTABLE)
        #######################
        # Shaders compilation #
        #######################

        set(SHADERS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

        # Recovering shaders
        file(
            GLOB_RECURSE
            CLOUD_RENDERING_SHADERS

            ${SHADERS_ROOT}/*.vert # Vertex shader
            ${SHADERS_ROOT}/*.tesc # Tesselation control shader
            ${SHADERS_ROOT}/*.tese # Tesselation evaluation shader
            ${SHADERS_ROOT}/*.geom # Geometry shader
            ${SHADERS_ROOT}/*.frag # Fragment shader
            ${SHADERS_ROOT}/*.comp # Compute shader
        )

        # Compiling all shaders found
        foreach (SHADER_INPUT_PATH ${CLOUD_RENDERING_SHADERS})
            get_filename_component(SHADER_FILENAME ${SHADER_INPUT_PATH} NAME) # Stripping the path from the prepending folders, keeping the file's name
            string(REPLACE "." ";" SHADER_FILE_PARTS ${SHADER_FILENAME})

            list(GET SHADER_FILE_PARTS 0 SHADER_NAME)
            list(GET SHADER_FILE_PARTS 1 SHADER_EXT)

            set(SHADER_OUTPUT_PATH "${SHADERS_ROOT}/build/${SHADER_NAME}_${SHADER_EXT}.spv")

            add_custom_command(
                OUTPUT "${SHADER_OUTPUT_PATH}"
                COMMAND ${GLSLC_EXECUTABLE} ${SHADER_INPUT_PATH} -o ${SHADER_OUTPUT_PATH}
                DEPENDS "${SHADER_INPUT_PATH}"
                WORKING_DIRECTORY "${SHADERS_ROOT}"
                COMMENT "Compiling shader ${SHADER_FILENAME} to SPIR-V"
                VERBATIM
            )

            target_sources(CloudRendering PRIVATE "${SHADER_OUTPUT_PATH}")
        endforeach ()
    else ()
        # GLSLC executable not found, send a warning

        set(GLSLC_EXECUTABLE "" CACHE PATH "Path to your GLSLC executable")
        message(WARNING "[Olympus] Warning: GLSLC executable not found. To compile shaders into SPIR-V, please specify its path manually.")
    endif ()
else ()
    # Neither method has found a match, send an error

    set(VULKAN_SDK_ROOT "" CACHE PATH "Path to your Vulkan SDK installation")
    message(SEND_ERROR "[Olympus] Error: Vulkan SDK not found, please specify its installation path manually.")
endif ()

##################################
# CLOUD_RENDERING - Source files #
##################################

set(
    CLOUD_RENDERING_FILES

    sources/*.cpp
    include/*.h
    include/*.inl
)   
   
# Adding every file to be compiled
file(
    GLOB_RECURSE
    CLOUD_RENDERING_SOURCES 
  
    ${CLOUD_RENDERING_FILES}
) 

##########################
# CloudRendering - Build #
##########################

target_sources(CloudRendering PRIVATE ${CLOUD_RENDERING_SOURCES})

target_include_directories(
	CloudRendering

	PRIVATE

	${CMAKE_CURRENT_SOURCE_DIR}/include
)
    
add_subdirectory(extern)
add_subdirectory(extern/glm)
set(
    CLOUD_RENDERING_LINKER_FLAGS 

    ${CLOUD_RENDERING_LINKER_FLAGS}
    GLFW 
    ImGui
    glm::glm
    Olympus
)
 
target_compile_definitions(
    CloudRendering
  
    PRIVATE  

    CLOUD_RENDERING_SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/shaders/build"
    GLFW_INCLUDE_VULKAN #use vulkan with GLFW
    GLM_ENABLE_EXPERIMENTAL # To be able to use quaternions
    GLM_FORCE_RADIANS # Forcing calculations to be in radians
    GLM_FORCE_DEPTH_ZERO_TO_ONE # To fix glm's perspective matrix
    #GLM_FORCE_LEFT_HANDED # Needs to be forced to LH for Vulkan
) 
 
target_compile_options(CloudRendering PRIVATE ${CLOUD_RENDERING_COMPILER_FLAGS})
target_link_libraries(CloudRendering PRIVATE ${CLOUD_RENDERING_LINKER_FLAGS})
  