# Stages io_scene_nifly the way the upstream release zip ships it (no VS project files,
# no caches), adds the dylib, and zips it. Run by the `addon` target.
file(REMOVE_RECURSE "${STAGE}")
file(COPY "${SRC}" DESTINATION "${STAGE}"
    PATTERN "__pycache__" EXCLUDE
    PATTERN "*.pyproj*"   EXCLUDE
    PATTERN "*.sln"       EXCLUDE
    PATTERN "*~"          EXCLUDE
    PATTERN "*.dll"       EXCLUDE)
file(COPY "${LIB}" DESTINATION "${STAGE}/io_scene_nifly")
file(REMOVE "${ZIP}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cf "${ZIP}" --format=zip io_scene_nifly
    WORKING_DIRECTORY "${STAGE}"
    COMMAND_ERROR_IS_FATAL ANY)
