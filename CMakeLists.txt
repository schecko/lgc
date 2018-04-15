cmake_minimum_required (VERSION 3.6)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

macro(set_out_dir dir)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${dir})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${dir})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${dir})

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${dir})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${dir})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${dir})

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${dir})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${dir})
endmacro()

set(LGC_VERSION_MAJOR 0)
set(LGC_VERSION_MINOR 0)
set(LGC_VERSION_PATCH 1)

project(LGC)

# add source
file(GLOB SOURCES "src/*.cpp" "src/*.h")

# configure output location
set(EXE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/built)

# declare binary
set_out_dir(${EXE_OUTPUT_DIR})
add_executable(THE_EXE ${SOURCES})

# configure compiler/linker

set (LGC_ARCHITECTURE "x64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set_target_properties(  THE_EXE PROPERTIES
                        OUTPUT_NAME lgc

                        SUFFIX ".exe"
                        
                        CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DLL_OUTPUT_DIR}
                        CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${DLL_OUTPUT_DIR}
                        CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${DLL_OUTPUT_DIR}

                        CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DLL_OUTPUT_DIR}
                        CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${DLL_OUTPUT_DIR}
                        CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${DLL_OUTPUT_DIR}

                        CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DLL_OUTPUT_DIR}
                        CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${DLL_OUTPUT_DIR}
                        CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${DLL_OUTPUT_DIR}
                    )
