set(TARGET_NAME ${PROJECT_NAME}-filter-flags)

include(RoqAutogen)

set(NAMESPACE "roq/cme/filter/flags")

set(AUTOGEN_SCHEMAS flags.json cme.json)

if(BUILD_DOCS)

  roq_autodoc(
    OUTPUT
    AUTOGEN_DOCS
    NAMESPACE
    ${NAMESPACE}
    OUTPUT_TYPE
    "doc"
    SOURCES
    ${AUTOGEN_SCHEMAS})

  add_custom_target(filter-autogen-docs ALL DEPENDS ${AUTOGEN_DOCS})

endif()

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  ${NAMESPACE}
  OUTPUT_TYPE
  "state"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(filter-autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  ${NAMESPACE}
  OUTPUT_TYPE
  "state"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(
  OUTPUT
  .gitignore
  SOURCES
  ${TARGET_NAME}
  ${AUTOGEN_DOCS}
  ${AUTOGEN_HEADERS}
  ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} filter-autogen-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-api::roq-api absl::flags)
