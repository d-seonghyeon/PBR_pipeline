cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0077 NEW)

set(PROJECT_NAME CreateWindow)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

# 1. 경로 변수 미리 설정 (가장 중요!)
set(SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shader")
set(IMAGE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/image")

# 2. 파일 스캔 (CONFIGURE_DEPENDS 추가로 파일 추가 시 자동 갱신)
file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS "${SHADER_SRC_DIR}/*.hlsl")
file(GLOB_RECURSE IMAGE_SOURCES CONFIGURE_DEPENDS "${IMAGE_SRC_DIR}/*.jpg" "${IMAGE_SRC_DIR}/*.png")

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 3. 실행 파일 생성
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/buffer.h src/buffer.cpp
    src/common.h 
    src/context.h src/context.cpp
    src/program.h src/program.cpp
    src/shader.h src/shader.cpp
    src/vertex_layout.h src/vertex_layout.cpp
    src/image.h src/image.cpp
    src/texture.h src/texture.cpp

    ${SHADER_SOURCES} 
    ${IMAGE_SOURCES}
)

# 4. [수정 포인트] 복사 명령 (변수 이름을 SHADER_SRC_DIR로 통일)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shader"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/image"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_SRC_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shader"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${IMAGE_SRC_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/image"
    COMMENT "VS Code: Syncing assets to build directory..."
    VERBATIM
)

# 외부 의존성 및 라이브러리 설정 (기존과 동일)
include(Dependency.cmake)

target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})

target_compile_definitions(${PROJECT_NAME} PUBLIC
    WINDOW_NAME="CreateWindow"
    WINDOW_WIDTH=960
    WINDOW_HEIGHT=540
)

set(DX_LIBS d3d11.lib dxgi.lib d3dcompiler.lib)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${DEP_LIBS} 
    ${DX_LIBS}
)

add_dependencies(${PROJECT_NAME} ${DEP_LIST})