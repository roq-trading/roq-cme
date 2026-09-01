set(TARGET_NAME ${PROJECT_NAME}-import)

add_subdirectory(flags)

set(SOURCES application.cpp controller.cpp settings.cpp main.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-import-flags
          ${PROJECT_NAME}-market_data
          ${PROJECT_NAME}-protocol-mdp
          ${PROJECT_NAME}-secdef
          ${PROJECT_NAME}-tools
          roq-api::roq-api
          roq-core::roq-core
          roq-codec::roq-codec
          roq-flags::roq-flags
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-utils::roq-utils
          EXPAT::EXPAT
          fmt::fmt)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(
  ${TARGET_NAME} PRIVATE ROQ_PACKAGE_NAME="${PROJECT_NAME}" ROQ_HOST="${ROQ_HOST}" ROQ_BUILD_VERSION="${GIT_REPO_VERSION}"
                         ROQ_GIT_DESCRIBE_HASH="${GIT_DESCRIBE_HASH}" ROQ_BUILD_NUMBER="${ROQ_BUILD_NUMBER}" ROQ_BUILD_TYPE="${ROQ_BUILD_TYPE}")

install(TARGETS ${TARGET_NAME})
