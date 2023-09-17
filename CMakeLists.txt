cmake_minimum_required (VERSION 3.12)

project (tebot C)

include (FindPkgConfig)

add_subdirectory (libcreqhttp)
add_library (tebot SHARED src/tebot.c)

pkg_check_modules (JSON "json-c")
pkg_check_modules (CURL "libcurl")

include_directories (tebot PUBLIC
	"include"
	"libcreqhttp/include"
	${JSON_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	)

target_link_libraries (tebot PUBLIC
	creqhttp
	${JSON_LIBRARIES}
	${CURL_LIBRARIES}
	)

install (TARGETS tebot)
