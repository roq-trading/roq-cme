set(TARGET_NAME "${PROJECT_NAME}-ilink")

set(SOURCES config_reader.cpp establish.cpp negotiate.cpp parser.cpp)

include(RoqAutogen)

add_library("${TARGET_NAME}" STATIC ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
