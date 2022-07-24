set(TARGET_NAME "${PROJECT_NAME}-fix")

set(SOURCES adv_order_type.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    affected_ord.json
    execution_report.json
    fill.json
    heartbeat.json
    instrmt_md_req.json
    instrument.json
    logon.json
    logout.json
    market_data_incremental_refresh.json
    market_data_request.json
    market_data_request_reject.json
    market_data_snapshot_full_refresh.json
    md_full.json
    md_inc.json
    md_req.json
    new_order_single.json
    order_cancel_reject.json
    order_cancel_replace_request.json
    order_cancel_request.json
    order_mass_cancel_report.json
    order_mass_cancel_request.json
    order_mass_status_request.json
    position_qty.json
    position_report.json
    reject.json
    request_for_positions.json
    resend_request.json
    security_list.json
    security_list_request.json
    security_status.json
    security_status_request.json
    sequence_reset.json
    test_request.json
    user_request.json
    user_response.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/deribit/fix"
  OUTPUT_TYPE
  "fix"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-fix-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/deribit/fix"
  OUTPUT_TYPE
  "fix"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" STATIC ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-fix-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
