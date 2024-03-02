set(TARGET_NAME ${PROJECT_NAME}-market_data)

set(SOURCES
    channel.cpp
    incremental.cpp
    instrument_definition.cpp
    manager.cpp
    market_by_order_recovery.cpp
    market_by_price_recovery.cpp
    shared.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-tools ${PROJECT_NAME}-mdp
                                             roq-market::roq-market fmt::fmt)
