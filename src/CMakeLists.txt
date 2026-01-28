add_library(core STATIC
  dummy.cpp
)

target_include_directories(core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
