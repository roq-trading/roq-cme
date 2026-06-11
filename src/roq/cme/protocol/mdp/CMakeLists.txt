set(TARGET_NAME ${PROJECT_NAME}-protocol-mdp)

set(SOURCES config.cpp config_reader.cpp frame.cpp map.cpp md_instrument_definition.cpp parser.cpp)

include(RoqAutogen)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-io::roq-io roq-utils::roq-utils roq-logging::roq-logging EXPAT::EXPAT fmt::fmt)
