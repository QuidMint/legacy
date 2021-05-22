set(CONTRACT_NAME eosio.token)

add_contract(${CONTRACT_NAME} ${CONTRACT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/${CONTRACT_NAME}.cpp)

target_include_directories(${CONTRACT_NAME}.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(${CONTRACT_NAME}.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/${CONTRACT_NAME}.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/${CONTRACT_NAME}.contracts.md @ONLY )

target_compile_options( ${CONTRACT_NAME}.wasm PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR} )

install(FILES
            ${CMAKE_BINARY_DIR}/contracts/${CONTRACT_NAME}/${CONTRACT_NAME}.abi
            ${CMAKE_BINARY_DIR}/contracts/${CONTRACT_NAME}/${CONTRACT_NAME}.wasm
            ${CMAKE_BINARY_DIR}/contracts/${CONTRACT_NAME}/${CONTRACT_NAME}.contracts.md
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/contracts/${CONTRACT_NAME}
)
