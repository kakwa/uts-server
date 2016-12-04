cmake_minimum_required (VERSION 2.6)
project (uts-server)
include(ExternalProject)


set(VERSION 0.1.2)

option(DEBUG     "compile with debug symbol"    OFF)
option(BUNDLE_CIVETWEB     "bundle civetweb with uts-server"    OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700")

if(DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
    set(CMAKE_BUILD_TYPE Debug)
endif(DEBUG)


add_custom_target(tag 
    git tag -a "${VERSION}" -m "version ${VERSION}" &&
    git push origin ${VERSION}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUTS_VERSION='\"${VERSION}\"'")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")


if(BUNDLE_CIVETWEB)
    ExternalProject_Add( civetweb
       GIT_REPOSITORY https://github.com/civetweb/civetweb
       GIT_TAG v1.8
       PATCH_COMMAND sed -i s/__DATE__/"110973"/ src/main.c
       INSTALL_DIR /usr/local
       CMAKE_ARGS .. -DCMAKE_INSTALL_PREFIX=/usr
        -DCIVETWEB_DISABLE_CGI=ON
        -DCIVETWEB_ENABLE_CXX=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DCIVETWEB_DISABLE_CGI=ON
        -DCIVETWEB_SERVE_NO_FILES=ON
        -DBUILD_TESTING=OFF
       INSTALL_COMMAND make install DESTDIR=${CMAKE_CURRENT_SOURCE_DIR}/vendor/
    )
    set(LIBCIVETWEB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/usr/include")
    set(LIBCIVETWEB_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/usr/lib/libcivetweb.a")
    set (CMAKE_C_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl -lpthread -lrt")
else(BUNDLE_CIVETWEB)
    find_package(libcivetweb REQUIRED)
endif(BUNDLE_CIVETWEB)

find_package(OpenSSL REQUIRED)
MESSAGE(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
MESSAGE(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")

MESSAGE( STATUS "Find Header Directory for libcivetweb: " ${LIBCIVETWEB_INCLUDE_DIR})
    MESSAGE( STATUS "Find Dynamic Library for libcivetweb:  " ${LIBCIVETWEB_LIBRARIES})
include_directories(
    ./inc/
    ${LIBCIVETWEB_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

add_executable(uts-server
    src/cmd/uts-server.c
    src/lib/rfc3161.c
    src/lib/http.c
    src/lib/utils.c
)

target_link_libraries(uts-server
    ${OPENSSL_LIBRARIES}
    ${LIBCIVETWEB_LIBRARIES}
)

INSTALL(TARGETS uts-server
    RUNTIME DESTINATION bin
)


if(BUNDLE_CIVETWEB)
    add_dependencies(uts-server civetweb)
endif(BUNDLE_CIVETWEB)
