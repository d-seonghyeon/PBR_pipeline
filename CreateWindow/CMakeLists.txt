# CMake 최소 버전 지정
cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.13...4.1)


# 프로젝트 이름 및 C++ 표준 설정
set(PROJECT_NAME CreateWindow) #프로젝트 이름
set(CMAKE_CXX_STANDARD 17)

set(WINDOW_NAME "CreateWindow")
set(WINDOW_WIDTH 960)
set(WINDOW_HEIGHT 540)

project(${PROJECT_NAME})
# 실행 파일과 기타파일이 build/bin 폴더 안에만 생기게 합니다.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if(MSVC)
    # 소스 파일과 실행 파일의 코드 페이지를 모두 UTF-8로 설정 (/utf-8)
    add_compile_options(/utf-8)
endif()


add_executable(${PROJECT_NAME} 
    src/main.cpp
)

# 외부 의존성 불러오기
include(Dependency.cmake)

# include / lib 경로 연결
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

#
target_compile_definitions(${PROJECT_NAME} PUBLIC

    WINDOW_NAME="CreateWindow"
    WINDOW_WIDTH=${WINDOW_WIDTH}
    WINDOW_HEIGHT=${WINDOW_HEIGHT}

)

# 의존성 빌드 후 실행파일 빌드
add_dependencies(${PROJECT_NAME} ${DEP_LIST})