if(BUILD_TESTING)
  add_subdirectory(test)
endif()

if(BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()

set(TARGET_NAME ${PROJECT_NAME})

add_subdirectory(flags)
add_subdirectory(tools)
add_subdirectory(mdp)
add_subdirectory(ilink)
add_subdirectory(secdef)
add_subdirectory(market_data)

add_subdirectory(channel_tester)
add_subdirectory(pcap_filter)
add_subdirectory(pcap_import)

set(SOURCES
    account.cpp
    application.cpp
    channel.cpp
    config.cpp
    gateway.cpp
    order_entry.cpp
    settings.cpp
    shared.cpp
    udp_incremental.cpp
    udp_instrument_definition.cpp
    udp_mbo_market_recovery.cpp
    udp_mbp_market_recovery.cpp
    main.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS order_entry_state.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/cme"
  OUTPUT_TYPE
  "state"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS})

add_executable(${TARGET_NAME} ${SOURCES})

add_dependencies(${TARGET_NAME} autogen-headers)

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-flags
          ${PROJECT_NAME}-tools
          ${PROJECT_NAME}-mdp
          ${PROJECT_NAME}-ilink
          ${PROJECT_NAME}-secdef
          ${PROJECT_NAME}-market_data
          roq-server::roq-server
          roq-server::roq-server-flags
          roq-web::roq-web
          roq-core::roq-core
          roq-io::roq-io
          roq-market::roq-market
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-flags::roq-flags
          roq-api::roq-api
          unordered_dense::unordered_dense
          EXPAT::EXPAT
          ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(
  ${TARGET_NAME}
  PRIVATE ROQ_PACKAGE_NAME="${TARGET_NAME}" ROQ_HOST="${ROQ_HOST}"
          ROQ_BUILD_VERSION="${GIT_REPO_VERSION}" ROQ_GIT_DESCRIBE_HASH="${GIT_DESCRIBE_HASH}"
          ROQ_BUILD_NUMBER="${ROQ_BUILD_NUMBER}" ROQ_BUILD_TYPE="${ROQ_BUILD_TYPE}")

install(TARGETS ${TARGET_NAME})
