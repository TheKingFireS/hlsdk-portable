#
# Cg 1.2 compatibility symbols are compiled into client.dll. This preserves
# legacy source/link compatibility only; the shim does not compile Cg shaders.
#
list(APPEND CLDLL_SOURCES
	cg_compat.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
