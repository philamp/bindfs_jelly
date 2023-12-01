cmake_minimum_required(VERSION 3.10)
project(bindfs-jelly-sqlite)

find_package(PkgConfig REQUIRED)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckStructHasMember)

#-------------------- FIND FUSE
find_path(FUSE_INCLUDE_DIR NAMES fuse3/fuse.h)
find_library(FUSE_LIBRARY NAMES fuse3)
if (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)
    set(FUSE_LIBRARIES ${FUSE_LIBRARY})
    set(FUSE_INCLUDE_DIRS ${FUSE_INCLUDE_DIR}/fuse3)
    set(FUSE_USE_VERSION 34)
    set(HAVE_FUSE_3 1)
    set(HAVE_FUSE_29 1)
    set(HAVE_FUSE_T 1)
    message(STATUS "Found fuse3 I=${FUSE_INCLUDE_DIRS} L=${FUSE_LIBRARIES}")
else ()
    # for now, let's crash if fuse3 is not there.
    message(FATAL_ERROR "Fuse3 not found... please apt install libfuse3-dev (I=${FUSE_INCLUDE_DIR} L=${FUSE_LIBRARY})")
endif ()

#-------- find sqlite3
find_path(SQLITE3_INCLUDE NAMES sqlite3.h)
find_library(SQLITE3_LIBRARY NAMES sqlite3)
if (SQLITE3_INCLUDE AND SQLITE3_LIBRARY)
    message(STATUS "Found sqlite3 I=${SQLITE3_INCLUDE} L=${SQLITE3_LIBRARY}")
else()
    message(FATAL_ERROR "Sqlite3 not found... please apt install libsqlite3-dev (I=${SQLITE3_INCLUDE} L=${SQLITE3_LIBRARY})")
endif()

#-------- check dlfcn
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdio.h HAVE_STDIO_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILE(sys/file.h HAVE_SYS_FILE_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
check_symbol_exists(getxattr sys/xattr.h HAVE_GETXATTR)
check_symbol_exists(lgetxattr sys/xattr.h HAVE_LGETXATTR)
check_symbol_exists(listxattr sys/xattr.h HAVE_LISTXATTR)
check_symbol_exists(llistxattr sys/xattr.h HAVE_LLISTXATTR)
check_symbol_exists(lremovexattr sys/xattr.h HAVE_LREMOVEXATTR)
check_symbol_exists(lsetxattr sys/xattr.h HAVE_LSETXATTR)
check_symbol_exists(lutimes sys/time.h HAVE_LUTIMES)
check_symbol_exists(utimensat "sys/stat.h;fcntl.h" HAVE_UTIMENSAT)
check_symbol_exists(removexattr sys/xattr.h HAVE_REMOVEXATTR)
check_symbol_exists(setxattr sys/xattr.h HAVE_SETXATTR)
CHECK_STRUCT_HAS_MEMBER(
  "struct stat" st_mtim.tv_nsec sys/stat.h
  HAVE_STAT_NANOSEC LANGUAGE C)

#-------- create config.h
configure_file(src/config.cmake.in src/config.h @ONLY)

#--------- Now the real stuff

add_library(supercollate SHARED sqlite_ext/supercollate.c)

set(BINDFS_SOURCES
        src/arena.c
        src/arena.h
        src/bindfs.c
        src/debug.c
        src/debug.h
        src/misc.c
        src/misc.h
        src/permchain.c
        src/permchain.h
        src/rate_limiter.c
        src/rate_limiter.h
	#src/sqlite3.c
	#src/sqlite3.h
        src/userinfo.c
        src/userinfo.h
        src/usermap.c
        src/usermap.h
)

add_executable(bindfs ${BINDFS_SOURCES})
target_include_directories(bindfs PRIVATE
    ${CMAKE_BINARY_DIR}/src
    ${FUSE_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE})
target_compile_definitions(bindfs PRIVATE 
    ${FUSE_DEFINITIONS})
target_link_libraries(bindfs 
    ${FUSE_LIBRARIES} 
    ${SQLITE3_LIBRARY}
    pthread)

install(TARGETS bindfs DESTINATION bin)
install(TARGETS supercollate DESTINATION share/bindfs-jelly)
