set(TARGET_NAME ${PROJECT_NAME}-gateway)

set(SOURCES
    account.cpp
    channel.cpp
    config.cpp
    controller.cpp
    mdp_receiver.cpp
    order_entry.cpp
    shared.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCES})

add_dependencies(${TARGET_NAME} flags-autogen-headers)

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-flags
          ${PROJECT_NAME}-market_data
          ${PROJECT_NAME}-protocol-ilink
          ${PROJECT_NAME}-protocol-mdp
          ${PROJECT_NAME}-secdef
          ${PROJECT_NAME}-tools
          roq-api::roq-api
          roq-core::roq-core
          roq-codec::roq-codec
          roq-flags::roq-flags
          roq-io::roq-io
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-market::roq-market
          roq-server::roq-server
          roq-web::roq-web
          unordered_dense::unordered_dense)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE ROQ_PACKAGE_NAME="${PROJECT_NAME}" ROQ_BUILD_VERSION="${GIT_REPO_VERSION}")

install(TARGETS ${TARGET_NAME})
