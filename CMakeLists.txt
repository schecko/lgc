cmake_minimum_required (VERSION 2.6)
project (LGC)
# The version number.
set (LGC_VERSION_MAJOR 1)
set (LGC_VERSION_MINOR 0)
 
SET(SOURCE "src/*.cpp")

# add the executable
add_executable(Tutorial ${SOURCE})