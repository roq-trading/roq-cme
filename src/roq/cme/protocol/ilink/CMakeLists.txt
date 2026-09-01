set(TARGET_NAME ${PROJECT_NAME}-protocol-ilink)

set(SOURCES
    config_reader.cpp
    establish.cpp
    mass_quote.cpp
    negotiate.cpp
    new_order_single.cpp
    order_cancel_replace_request.cpp
    order_cancel_request.cpp
    order_mass_action_request.cpp
    order_mass_status_request.cpp
    parser.cpp
    party_details_definition_request.cpp
    party_details_list_request.cpp
    quote_cancel.cpp
    security_definition_request.cpp
    sequence.cpp
    terminate.cpp)

include(RoqAutogen)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-io::roq-io roq-logging::roq-logging EXPAT::EXPAT fmt::fmt)
