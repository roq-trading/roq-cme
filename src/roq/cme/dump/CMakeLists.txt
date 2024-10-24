set(TARGET_NAME ${PROJECT_NAME}-dump)

add_subdirectory(flags)

set(SOURCES application.cpp controller.cpp pcap.cpp settings.cpp main.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-dump-flags
          ${PROJECT_NAME}-mdp
          roq-core::roq-core
          roq-utils::roq-utils
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-flags::roq-flags
          roq-api::roq-api
          fmt::fmt
          EXPAT::EXPAT
          ${LIBPCAP_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(
  ${TARGET_NAME} PRIVATE ROQ_PACKAGE_NAME="${TARGET_NAME}" ROQ_HOST="${ROQ_HOST}" ROQ_BUILD_VERSION="${GIT_REPO_VERSION}"
                         ROQ_GIT_DESCRIBE_HASH="${GIT_DESCRIBE_HASH}" ROQ_BUILD_NUMBER="${ROQ_BUILD_NUMBER}" ROQ_BUILD_TYPE="${ROQ_BUILD_TYPE}")

install(TARGETS ${TARGET_NAME})
