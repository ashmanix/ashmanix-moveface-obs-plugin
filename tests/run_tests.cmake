set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# include("${BASE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

# message("Base Directory: ${BASE_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

include("${BASE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

execute_process(
    COMMAND mkdir "${CMAKE_CURRENT_SOURCE_DIR}/tests/build"
    COMMAND cmake -S ./tests -B tests/build
    # COMMAND cmake --build "${CMAKE_CURRENT_SOURCE_DIR}/tests/build" --target test --config Release
    RESULT_VARIABLE test_result
)