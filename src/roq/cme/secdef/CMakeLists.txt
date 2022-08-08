set(TARGET_NAME "${PROJECT_NAME}-secdef")

set(SOURCES config_reader.cpp)

add_library("${TARGET_NAME}" STATIC ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-core::roq-core fmt::fmt)
