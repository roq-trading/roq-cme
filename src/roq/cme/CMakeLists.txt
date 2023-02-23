set(TARGET_NAME "${PROJECT_NAME}")

add_subdirectory("flags")
add_subdirectory("mdp3")
add_subdirectory("ilink3")
add_subdirectory("multicast")
add_subdirectory("secdef")
add_subdirectory("channel_tester")

set(SOURCES
    application.cpp
    channel.cpp
    config.cpp
    gateway.cpp
    security.cpp
    shared.cpp
    udp_incremental.cpp
    udp_instrument_definition.cpp
    udp_mbp_market_recovery.cpp
    udp_mbo_market_recovery.cpp
    main.cpp)

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}")

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries(
  "${TARGET_NAME}"
  PRIVATE "${PROJECT_NAME}-flags"
          "${PROJECT_NAME}-mdp3"
          "${PROJECT_NAME}-ilink3"
          "${PROJECT_NAME}-multicast"
          "${PROJECT_NAME}-secdef"
          roq-server::roq-server
          roq-web::roq-web
          roq-io::roq-io
          roq-core::roq-core
          roq-logging::roq-logging
          roq-api::roq-api
          absl::flags
          absl::flat_hash_map
          absl::flat_hash_set
          aklomp::base64
          EXPAT::EXPAT
          ${UUID_LIBRARIES}
          ${RT_LIBRARIES})

target_compile_features("${TARGET_NAME}" PUBLIC cxx_std_20)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

target_compile_definitions(
  "${TARGET_NAME}"
  PRIVATE ROQ_PACKAGE_NAME="${TARGET_NAME}" ROQ_HOST="${ROQ_HOST}"
          ROQ_BUILD_VERSION="${GIT_REPO_VERSION}" ROQ_GIT_DESCRIBE_HASH="${GIT_DESCRIBE_HASH}"
          ROQ_BUILD_NUMBER="${ROQ_BUILD_NUMBER}" ROQ_BUILD_TYPE="${ROQ_BUILD_TYPE}")

install(TARGETS ${TARGET_NAME})
