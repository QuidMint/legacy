cmake_minimum_required( VERSION 3.5 )

set(CONTRACT_NAME curve.sx)
set(INSTALL_DIR_NAME vigorsxcurve)

add_contract(${CONTRACT_NAME} ${CONTRACT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${CONTRACT_NAME}.cpp)

target_include_directories(${CONTRACT_NAME}.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
