cmake_minimum_required( VERSION 3.5 )

set(CONTRACT_NAME zap.sx)
set(INSTALL_DIR_NAME vigorsxzap11)
get_filename_component(PARENT_DIR ../ ABSOLUTE)
include_directories(${PARENT_DIR}/vigorsxcurve/include)
include_directories(${PARENT_DIR}/vigorsxcurve)

add_contract(${CONTRACT_NAME} ${CONTRACT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${CONTRACT_NAME}.cpp)

set_target_properties(${CONTRACT_NAME}.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options(${CONTRACT_NAME}.wasm PUBLIC -R${CMAKE_CURRENT_BINARY_DIR} )

install(FILES
            ${CMAKE_BINARY_DIR}/contracts/${INSTALL_DIR_NAME}/${CONTRACT_NAME}.abi
            ${CMAKE_BINARY_DIR}/contracts/${INSTALL_DIR_NAME}/${CONTRACT_NAME}.wasm
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/contracts/${INSTALL_DIR_NAME}
)
