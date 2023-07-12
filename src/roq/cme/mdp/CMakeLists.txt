set(TARGET_NAME ${PROJECT_NAME}-mdp)

set(SOURCES config.cpp config_reader.cpp frame.cpp md_instrument_definition.cpp parser.cpp)

include(RoqAutogen)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} roq-core::roq-core roq-logging::roq-logging fmt::fmt
                      EXPAT::EXPAT)
