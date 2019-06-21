cmake_minimum_required(VERSION 2.8)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(p-thread)

enable_language(CXX C ASM)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
###################################################################################

# file(WRITE ${PROJECT_BINARY_DIR}/a.c "void a(void){}\n")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../p-base/include
    )

list(APPEND P_THREAD_SRC
    src/asm/jump_pcontext.S
    src/asm/make_pcontext.S
    src/task.cpp
    src/task_worker.cpp
    src/worker_manager.cpp
    )

set(P_BASE_EXTERNAL_SOURCE_DIR ../p-base CACHE PATH "p-base")
add_subdirectory(${P_BASE_EXTERNAL_SOURCE_DIR} p-base)

add_library(p-thread STATIC ${P_THREAD_SRC})

