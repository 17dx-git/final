cmake_minimum_required(VERSION 2.6)

#cmake -G "Unix Makefiles" -D CMAKE_MAKE_PROGRAM=mingw32-make -D CMAKE_CXX_COMPILER=g++ .

if (EXISTS CMakeCache.txt)
     file(REMOVE CMakeCache.txt)
endif()
if (EXISTS CMakeFiles)
     file(REMOVE_RECURSE CMakeFiles)
endif()

project (final CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set (LIBS "-pthread")

set(SOURCE_EXE main.cpp server.cpp getopt.cpp query.cpp response.cpp)
enable_language(CXX)
#SET(CMAKE_EXE_LINKER_FLAGS "-lWs2_32")
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_EXE} )
target_link_libraries(${PROJECT_NAME} ${LIBS})
#add_custom_command(TARGET ${PROJECT_NAME} 
#                   POST_BUILD
#                   COMMAND ./final -h 127.0.0.1 -p 1080 -d .)
