cmake_minimum_required (VERSION 2.6)
project (uts-server)
include(ExternalProject)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
  endif ()
else ()
  set (CMAKE_C_STANDARD 99)
endif ()

set(VERSION 0.1.10)

option(DEBUG              "compile with debug symbol"    OFF)
option(BUNDLE_CIVETWEB    "bundle civetweb with uts-server"    OFF)
option(STATIC             "static linked binary" OFF)
option(LINK_DL            "link dl" OFF)
option(LINK_GCC_S         "link gcc_s" OFF)
option(CIVETWEB_CUST_ARGS "Custom args for civetweb (if civetweb is bundled)" "")
option(LINK_PTHREAD       "link pthread" OFF)

IF("$ENV{CIVETWEB_GITURL}" STREQUAL "")
    set(CIVETWEB_GITURL    "https://github.com/civetweb/civetweb")
ELSE()
    set(CIVETWEB_GITURL    "$ENV{CIVETWEB_GITURL}")
ENDIF()

IF("$ENV{CIVETWEB_GITTAG}" STREQUAL "")
    set(CIVETWEB_GITTAG    "v1.10")
ELSE()
    set(CIVETWEB_GITTAG    "$ENV{CIVETWEB_GITTAG}")
ENDIF()



IF(STATIC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    set(OPENSSL_USE_STATIC_LIBS ON)
    set(ARGP_USE_STATIC_LIBS ON)
    set(CIVETWEB_USE_STATIC_LIBS ON)
ENDIF(STATIC)

IF(BUNDLE_CIVETWEB)
    set(DL_LIBRARIES 'dl')
ENDIF(BUNDLE_CIVETWEB)

IF(LINK_DL)
    set(DL_LIBRARIES 'dl')
ENDIF(LINK_DL)

IF(LINK_GCC_S)
    set(GCC_S_LIBRARIES 'gcc_s')
ENDIF(LINK_GCC_S)

IF(LINK_PTHREAD)
    set(PTHREAD_LIBRARIES 'pthread')
ENDIF(LINK_PTHREAD)

set (CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700")

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

find_package(OpenSSL REQUIRED)
find_package(LibreSSL)

MESSAGE(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
MESSAGE(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
MESSAGE(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
MESSAGE(STATUS "LibreSSL Detected: ${IS_LIBRESSL}")
MESSAGE(STATUS "OS Detected: ${CMAKE_SYSTEM_NAME}")


if(NOT(DEFINED OPENSSL_API_1_1))
    if(${OPENSSL_VERSION} VERSION_GREATER 1.0.99 AND NOT(IS_LIBRESSL))
        set(OPENSSL_API_1_1 ON)
    else()
        set(OPENSSL_API_1_1 OFF)
    endif()
endif()


IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(SED_FREEBSD "*.c.bak")
    find_package(argp REQUIRED)
    add_definitions(-DBSD)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

IF(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
    find_package(argp REQUIRED)
    add_definitions(-DBSD)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")


if(BUNDLE_CIVETWEB)
    MESSAGE(STATUS "Building Civetweb From: ${CIVETWEB_GITURL}")
    ExternalProject_Add( civetweb
       GIT_REPOSITORY ${CIVETWEB_GITURL}
       GIT_TAG ${CIVETWEB_GITTAG}
       PATCH_COMMAND sed -i ${SED_FREEBSD} s/__DATE__/"110973"/ src/main.c src/civetweb.c
       INSTALL_DIR /usr/local
       CMAKE_ARGS .. -DCMAKE_INSTALL_PREFIX=/usr
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCIVETWEB_ENABLE_CXX=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DCIVETWEB_DISABLE_CGI=ON
        -DCIVETWEB_SERVE_NO_FILES=ON
        -DCIVETWEB_ENABLE_IPV6=ON
        -DCIVETWEB_DISABLE_CACHING=ON
        -DBUILD_TESTING=OFF
        -DCIVETWEB_SSL_OPENSSL_API_1_1=${OPENSSL_API_1_1}
        -DOPENSSL_USE_STATIC_LIBS=${OPENSSL_USE_STATIC_LIBS}
        -DCIVETWEB_ENABLE_SSL_DYNAMIC_LOADING=ON
        -DCIVETWEB_ALLOW_WARNINGS=ON
        ${CIVETWEB_CUST_ARGS}
       INSTALL_COMMAND make install DESTDIR=${CMAKE_CURRENT_SOURCE_DIR}/vendor/
    )
    set(LIBCIVETWEB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/usr/include")
    set(LIBCIVETWEB_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/usr/lib/libcivetweb.a")
    IF(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
        set (CMAKE_C_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -lpthread")
    ELSE(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
        set (CMAKE_C_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -lpthread -lrt")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
else(BUNDLE_CIVETWEB)
    find_package(libcivetweb REQUIRED)
endif(BUNDLE_CIVETWEB)

MESSAGE( STATUS "Find Header Directory for libcivetweb: " ${LIBCIVETWEB_INCLUDE_DIR})
    MESSAGE( STATUS "Find Dynamic Library for libcivetweb:  " ${LIBCIVETWEB_LIBRARIES})
include_directories(
    ./inc/
    ${LIBCIVETWEB_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${ARGP_INCLUDE_DIR}
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
    ${ARGP_LIBRARIES}
    ${DL_LIBRARIES}
    ${GCC_S_LIBRARIES}
    ${PTHREAD_LIBRARIES}
)

INSTALL(TARGETS uts-server
    RUNTIME DESTINATION bin
)

if(BUNDLE_CIVETWEB)
    add_dependencies(uts-server civetweb)
endif(BUNDLE_CIVETWEB)
