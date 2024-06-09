set(TARGET_NAME ${PROJECT_NAME}-channel-tester-flags)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS flags.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "state")

add_custom_target(channel-tester-autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "state")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} channel-tester-autogen-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-api::roq-api absl::flags)
