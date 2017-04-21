#.rst
# FindLibreSSL
# ------------
#
# Detect if OpenSSL is in fact LibreSSL, and recovers libressl version.
# 
# Requires running FindOpenSSL previously
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# ``LIBRESSL_VERSION``
# This is set to ``$major.$minor.$revision$patch`` (e.g. ``2.3.1f``).
#
# ``IS_LIBRESSL``
# Boolean, set to true if LibreSSL
#

if(OPENSSL_INCLUDE_DIR AND EXISTS "${OPENSSL_INCLUDE_DIR}/openssl/opensslv.h")
  file(STRINGS "${OPENSSL_INCLUDE_DIR}/openssl/opensslv.h" libressl_version_str
       REGEX "^#[\t ]*define[\t ]+LIBRESSL_VERSION_NUMBER[\t ]+0x([0-9a-fA-F])+.*")

  if(libressl_version_str)
    # The version number is encoded as 0xMNNFFPPS: major minor fix patch status
    # The status gives if this is a developer or prerelease and is ignored here.
    # Major, minor, and fix directly translate into the version numbers shown in
    # the string. The patch field translates to the single character suffix that
    # indicates the bug fix state, which 00 -> nothing, 01 -> a, 02 -> b and so
    # on.

    string(REGEX REPLACE "^.*LIBRESSL_VERSION_NUMBER[\t ]+0x([0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])([0-9a-fA-F]).*$"
           "\\1;\\2;\\3;\\4;\\5" LIBRESSL_VERSION_LIST "${libressl_version_str}")
    list(GET LIBRESSL_VERSION_LIST 0 LIBRESSL_VERSION_MAJOR)
    list(GET LIBRESSL_VERSION_LIST 1 LIBRESSL_VERSION_MINOR)
    from_hex("${LIBRESSL_VERSION_MINOR}" LIBRESSL_VERSION_MINOR)
    list(GET LIBRESSL_VERSION_LIST 2 LIBRESSL_VERSION_FIX)
    from_hex("${LIBRESSL_VERSION_FIX}" LIBRESSL_VERSION_FIX)
    list(GET LIBRESSL_VERSION_LIST 3 LIBRESSL_VERSION_PATCH)

    if (NOT LIBRESSL_VERSION_PATCH STREQUAL "00")
      from_hex("${LIBRESSL_VERSION_PATCH}" _tmp)
      # 96 is the ASCII code of 'a' minus 1
      math(EXPR LIBRESSL_VERSION_PATCH_ASCII "${_tmp} + 96")
      unset(_tmp)
      # Once anyone knows how OpenSSL would call the patch versions beyond 'z'
      # this should be updated to handle that, too. This has not happened yet
      # so it is simply ignored here for now.
      string(ASCII "${LIBRESSL_VERSION_PATCH_ASCII}" LIBRESSL_VERSION_PATCH_STRING)
    endif ()

    set(LIBRESSL_VERSION "${LIBRESSL_VERSION_MAJOR}.${LIBRESSL_VERSION_MINOR}.${LIBRESSL_VERSION_FIX}${LIBRESSL_VERSION_PATCH_STRING}")
    set(IS_LIBRESSL ON)
  else ()
    set(IS_LIBRESSL OFF)
  endif ()
endif ()
