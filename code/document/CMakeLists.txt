set(target document)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

make_library(${target} "STATIC")

add_sources(${target}
	"Document"
	"document.cpp"
	"document.h"
)

add_sources(${target}
	"Util"
	"position.h"
	"range.h"
	"type.h"
	"encoding.h"
)
