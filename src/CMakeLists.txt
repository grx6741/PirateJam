file(
    GLOB
    source_files
    CONFIGURE_DEPENDS
    "*.c"
    "*.h"
    "./util/*.h"
)

target_sources(
    ${CMAKE_PROJECT_NAME}
    PUBLIC
    ${source_files}
)
