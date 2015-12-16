cmake_minimum_required (VERSION 2.6)
project (uts-server)

set(VERSION 0.1.0)

add_custom_target(tag 
    git tag -a "${VERSION}" -m "version ${VERSION}" &&
    git push origin ${VERSION}
)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DGCCPROG_VERSION='\"${VERSION}\"'")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(libasyncd REQUIRED)

MESSAGE( STATUS "Find Header Directory for libasyncd: " ${LIBASYNCD_INCLUDE_DIR})
MESSAGE( STATUS "Find Dynamic Library for libasyncd:  " ${LIBASYNCD_LIBRARIES})

include_directories(
    ${LIBASYNCD_INCLUDE_DIR}
)

add_executable(uts-server src/cmd/uts-server.c)

target_link_libraries(uts-server  ${LIBASYNCD_LIBRARIES})

INSTALL(TARGETS uts-server
    RUNTIME DESTINATION bin
)
