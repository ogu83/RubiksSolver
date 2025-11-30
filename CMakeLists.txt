# CMakeList.txt : CMake project for RubiksSolver, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("RubiksSolver")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source to this project's executable.
add_executable (RubiksSolver "RubiksSolver.cpp" "RubiksSolver.h")

# Add test executable
add_executable (RubiksSolverTests "RubiksSolverTests.cpp" "RubiksSolver.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RubiksSolver PROPERTY CXX_STANDARD 20)
  set_property(TARGET RubiksSolverTests PROPERTY CXX_STANDARD 20)
endif()

# Enable testing
enable_testing()
add_test(NAME RubiksSolverTests COMMAND RubiksSolverTests)
