cmake_minimum_required(VERSION 3.0.0)
project(ELEN4020_Project VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)
find_package(netCDFCxx CONFIG REQUIRED)
add_definitions(-DOMPI_SKIP_MPICXX)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" -fopenmp)
include_directories(${PROJECT_SOURCE_DIR} SYSTEM ${MPI_INCLUDE_PATH})

add_executable(${PROJECT_NAME} main.cpp Array1D.cpp HelperFunctions.cpp FileReader.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${MPI_C_LIBRARIES} OpenMP::OpenMP_CXX netCDF::netcdf netCDF::netcdf-cxx4)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)