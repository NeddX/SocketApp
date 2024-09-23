cmake_minimum_required(VERSION 3.20)

project("SocketApp")

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if (NOT CMAKE_GENERATOR MATCHES "Visual Studio")
add_custom_command(
      OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json
              ${CMAKE_SOURCE_DIR}/compile_commands.json
      DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
      COMMENT "Copying compile_commands.json...")

    add_custom_target(copy_compile_commands ALL
                      DEPENDS ${CMAKE_SOURCE_DIR}/compile_commands.json)
else()
endif()

add_executable(server "server.cpp")
add_executable(client "client.cpp")

set_property(TARGET server PROPERTY CXX_STANDARD 20)
set_property(TARGET client PROPERTY CXX_STANDARD 20)
