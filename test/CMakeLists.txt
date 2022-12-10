set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES sbe.cpp config.cpp secdef.cpp main.cpp)

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}")

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries(
  "${TARGET_NAME}"
  "${PROJECT_NAME}-multicast"
  "${PROJECT_NAME}-secdef"
  "${PROJECT_NAME}-sbe"
  roq-server::roq-server
  roq-core::roq-core
  roq-io::roq-io
  roq-logging::roq-logging
  aklomp::base64
  EXPAT::EXPAT
  Catch2::Catch2
  ${UUID_LIBRARIES}
  ${RT_LIBRARIES})

target_compile_features("${TARGET_NAME}" PUBLIC cxx_std_20)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
