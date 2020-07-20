
project(dbus_demo)


find_package(PkgConfig) # Include functions provided by PkgConfig module.

pkg_check_modules(DBUS dbus-1) # This calls pkgconfig with appropriate arguments

include_directories(${DBUS_INCLUDE_DIRS})

add_executable(dbus_client dbus_client.cpp)
add_executable(dbus_server dbus_server.cpp)

link_directories(${DBUS_LIBRARY_DIRS})

target_link_libraries(dbus_client ${DBUS_LIBRARIES})
target_link_libraries(dbus_server ${DBUS_LIBRARIES})