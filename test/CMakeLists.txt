set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES
    establish.cpp
    ilink_config_reader.cpp
    ilink_simple.cpp
    incremental.cpp
    mbofd_recovery.cpp
    md_incremental_refresh_book.cpp
    md_incremental_refresh_trade_summary.cpp
    md_instrument_definition.cpp
    mdp_config_reader.cpp
    mdp.cpp
    negotiate.cpp
    new_order_single.cpp
    secdef.cpp
    snapshot_full_refresh.cpp
    tools_crypto.cpp
    main.cpp)

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}")

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries(
  "${TARGET_NAME}"
  "${PROJECT_NAME}-secdef"
  "${PROJECT_NAME}-mdp"
  "${PROJECT_NAME}-ilink"
  "${PROJECT_NAME}-tools"
  roq-server::roq-server
  roq-core::roq-core
  roq-io::roq-io
  roq-logging::roq-logging
  aklomp::base64
  EXPAT::EXPAT
  Catch2::Catch2
  ${UUID_LIBRARIES}
  ${RT_LIBRARIES})

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
