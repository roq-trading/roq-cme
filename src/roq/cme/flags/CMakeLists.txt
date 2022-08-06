set(TARGET_NAME "${PROJECT_NAME}-flags")

include(RoqAutogen)

set(AUTOGEN_SCHEMAS common.json config.json fix.json multicast.json web_socket.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/cme/flags"
  OUTPUT_TYPE
  "state"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(flags-autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/cme/flags"
  OUTPUT_TYPE
  "state"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" STATIC ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" flags-autogen-headers)

target_link_libraries("${TARGET_NAME}" absl::flags)

target_compile_features("${TARGET_NAME}" PUBLIC cxx_std_20)
