find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

add_library(ffmpeg INTERFACE)

target_link_libraries(ffmpeg INTERFACE
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${AVDEVICE_LIBRARY}
)

if(UNIX AND NOT APPLE)
    target_include_directories(ffmpeg INTERFACE
        /usr/include/ffmpeg
    )
elseif(APPLE)
    target_include_directories(ffmpeg INTERFACE
        /opt/homebrew/include
    )
else()
    message(FATAL "ffmpeg include directory not specificed on this platform yet!")
endif()