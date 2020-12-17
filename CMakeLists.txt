cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(demo_decode)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11")

# include(${PROJECT_SOURCE_DIR}/cmake/build_config.cmake)
# include(${PROJECT_SOURCE_DIR}/cmake/build_app.cmake)


include_directories(/home/duanxiang/demo/codec/3rdParty/cnrt/include)
include_directories(/home/duanxiang/demo/codec/3rdParty/cncodec/include)
include_directories(/home/duanxiang/demo/codec/3rdParty/ffmpeg/include)
include_directories(/home/duanxiang/demo/codec/3rdParty/cnrt/include)

link_directories(/home/duanxiang/demo/codec/3rdParty/cnrt/lib64)
link_directories(/home/duanxiang/demo/codec/3rdParty/cncodec/lib64)
link_directories(/home/duanxiang/demo/codec/3rdParty/cndrv/lib64)
link_directories(/home/duanxiang/demo/codec/3rdParty/ffmpeg/lib)


# build_demo_exec(jdecode_demo jpeg_decode)
# build_demo_exec(jencode_demo jpeg_encode)
# build_demo_exec(encode_demo  video_encode)
# build_demo_exec(decode_demo  video_decode)

include_directories(include)
aux_source_directory(src SRC_CODE)

add_executable(demo_decode ${SRC_CODE})

target_link_libraries(demo_decode cncodec cnrt cndrv pthread avformat avcodec avutil)
