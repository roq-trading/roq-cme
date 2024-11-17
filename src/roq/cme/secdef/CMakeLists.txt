set(TARGET_NAME ${PROJECT_NAME}-secdef)

set(SOURCES config_reader.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-io::roq-io fmt::fmt)
