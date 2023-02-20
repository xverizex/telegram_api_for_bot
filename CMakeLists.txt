cmake_minimum_required (VERSION 2.8.12)

project (tebot C)

include (FindPkgConfig)

add_library (tebot SHARED src/tebot.c)

pkg_check_modules (JSON "json-c")
pkg_check_modules (CURL "libcurl")

include_directories (tebot PUBLIC
	"include"
	${JSON_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	)

target_link_libraries (tebot PUBLIC
	${JSON_LIBRARIES}
	${CURL_LIBRARIES}
	)

install (TARGETS tebot)
