foreach(dependency SDL bgfx.cmake)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/${dependency}/CMakeLists.txt")
        message(FATAL_ERROR
            "Fetch the desktop dependencies with: git submodule update --init --recursive thirdparty/SDL thirdparty/bgfx.cmake")
    endif()
endforeach()

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_VIDEO ON CACHE BOOL "" FORCE)
foreach(subsystem AUDIO GPU RENDER CAMERA JOYSTICK HAPTIC HIDAPI POWER SENSOR DIALOG TRAY)
    set(SDL_${subsystem} OFF CACHE BOOL "" FORCE)
endforeach()
set(SDL_OPENGL OFF CACHE BOOL "" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_OFFSCREEN OFF CACHE BOOL "" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/SDL" "${PROJECT_BINARY_DIR}/thirdparty/SDL")

set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
set(BGFX_CONFIG_RENDERER_WEBGPU OFF CACHE BOOL "" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/thirdparty/bgfx.cmake" "${PROJECT_BINARY_DIR}/thirdparty/bgfx.cmake")
set_target_properties(bimg_decode bimg_encode PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_executable(OpenTSMacShell MACOSX_BUNDLE
    main.cpp
    diagnostics.cpp
    "${PROJECT_SOURCE_DIR}/code/bgfxbackend.cpp"
)
target_compile_features(OpenTSMacShell PRIVATE cxx_std_20)
target_include_directories(OpenTSMacShell PRIVATE
    "${PROJECT_SOURCE_DIR}/code"
    "${PROJECT_SOURCE_DIR}/thirdparty/bgfx.cmake/bgfx/examples/common/imgui"
)
target_link_libraries(OpenTSMacShell PRIVATE SDL3::SDL3-static bgfx bimg bx)
set_target_properties(OpenTSMacShell PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.tfrey.opents.mac.shell"
    MACOSX_BUNDLE_BUNDLE_NAME "OpenTS Mac Shell"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)
