set(TARGET_NAME "${PROJECT_NAME}-tools")

set(SOURCES secdef_reader.cpp)

add_library("${TARGET_NAME}" STATIC ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-core::roq-core fmt::fmt)
