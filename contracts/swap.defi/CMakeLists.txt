cmake_minimum_required( VERSION 3.5 )

set(CONTRACT_NAME swap.defi)
set(INSTALL_DIR_NAME swap.defi)

add_contract(${CONTRACT_NAME} ${CONTRACT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${CONTRACT_NAME}.cpp)

set_target_properties(${CONTRACT_NAME}.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ricardian/contract.md.in ${CMAKE_CURRENT_BINARY_DIR}/${CONTRACT_NAME}.md @ONLY )

target_compile_options(${CONTRACT_NAME}.wasm PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR} )

install(FILES
            ${CMAKE_BINARY_DIR}/contracts/${INSTALL_DIR_NAME}/${CONTRACT_NAME}.abi
            ${CMAKE_BINARY_DIR}/contracts/${INSTALL_DIR_NAME}/${CONTRACT_NAME}.wasm
            ${CMAKE_BINARY_DIR}/contracts/${INSTALL_DIR_NAME}/${CONTRACT_NAME}.md
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/contracts/${INSTALL_DIR_NAME}
)
