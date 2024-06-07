set(TARGET_NAME ${PROJECT_NAME}-market_data)

set(SOURCES
    channel.cpp
    incremental.cpp
    instrument_definition.cpp
    manager.cpp
    mbofd_market_recovery.cpp
    mbp_market_recovery.cpp
    security_definitions.cpp
    shared.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-tools ${PROJECT_NAME}-mdp roq-server::roq-server roq-market::roq-market fmt::fmt)
