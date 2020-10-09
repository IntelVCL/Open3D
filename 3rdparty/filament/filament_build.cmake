include(ExternalProject)

set(FILAMENT_ROOT "${CMAKE_BINARY_DIR}/filament-binaries")

if(LINUX_AARCH64)
    set(FILAMENT_GIT_TAG "d72d3533e66b7e6c10e27e4bb720317c23f7de6f") # 2020-Oct-05
else()
    set(FILAMENT_GIT_TAG "v1.9.3")
endif()

ExternalProject_Add(
    ext_filament
    PREFIX filament
    GIT_REPOSITORY https://github.com/google/filament.git
    GIT_TAG ${FILAMENT_GIT_TAG}
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DCCACHE_PROGRAM=OFF  # Enables ccache, "launch-cxx" is not working.
        -DFILAMENT_ENABLE_JAVA=OFF
        -DCMAKE_C_COMPILER=${FILAMENT_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${FILAMENT_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS="-fno-builtin"  # Issue Open3D#1909, filament#2146
        -DCMAKE_INSTALL_PREFIX=${FILAMENT_ROOT}
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DUSE_STATIC_CRT=${STATIC_WINDOWS_RUNTIME}
        -DUSE_STATIC_LIBCXX=ON
        -DFILAMENT_SUPPORTS_VULKAN=OFF
        -DFILAMENT_SKIP_SAMPLES=ON
)

set(filament_LIBRARIES
    filameshio
    filament
    filamat_lite
    filamat
    filaflat
    filabridge
    geometry
    backend
    bluegl
    ibl
    image
    meshoptimizer
    smol-v
    utils
)
