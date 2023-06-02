set(TARGET_NAME "${PROJECT_NAME}-ilink")

set(SOURCES
    config_reader.cpp
    establish.cpp
    negotiate.cpp
    new_order_single.cpp
    order_cancel_request.cpp
    order_mass_action_request.cpp
    order_mass_status_request.cpp
    parser.cpp
    party_details_definition_request.cpp
    party_details_list_request.cpp
    security_definition_request.cpp
    sequence.cpp
    terminate.cpp)

include(RoqAutogen)

add_library("${TARGET_NAME}" OBJECT ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
