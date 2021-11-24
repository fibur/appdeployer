cmake_minimum_required(VERSION 3.0.0 )
project(appdeployer)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(QT_VERSION_MAJOR 5)

file (GLOB_RECURSE SRC_FILES "*.cpp")
file (GLOB_RECURSE HDR_FILES "*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HDR_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SRC_FILES})

find_package(Qt${QT_VERSION_MAJOR} EXACT COMPONENTS REQUIRED Core Network Widgets)

add_executable(appdeployer
  ${SRC_FILES} ${HDR_FILES}
)

target_compile_definitions(appdeployer
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(appdeployer PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets)
