function(ConiferAddExecutable ConiferAddExecutable_EXECUTABLE ConiferAddExecutable_BUILD_RES)
    set(options IDENTIFIER COPYRIGHT)
    set(oneValueArgs NAME VERSION ICON_SVG RES_INTERNAL_DIR)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(ConiferAddExecutable "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # macOS specific code
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        # If an icon was provided
        if (ConiferAddExecutable_ICON_SVG)
            # Double check that a internal resource location was specified
            if (NOT ConiferAddExecutable_RES_INTERNAL_DIR)
                message(FATAL_ERROR "No internal resource directory specified for '${ConiferAddExecutable_EXECUTABLE}'. Use 'RES_INTERNAL_DIR'.")
            endif ()

            # Build icon from SVG
            RunOnceStallAllStartBlock(${ConiferAddExecutable_BUILD_RES}/.lock CONTINUE)
            if (CONTINUE)
                find_program(SVG_CONVERTER rsvg-convert REQUIRED)  # brew install librsvg
                find_program(ICON_UTIL iconutil REQUIRED)
                execute_process(COMMAND ${CMAKE_COMMAND} -E;make_directory;${ConiferAddExecutable_RES_INTERNAL_DIR}/icon.iconset)
                foreach (SIZE 16;32;64;128;256;512)
                    execute_process(COMMAND ${SVG_CONVERTER} -a;-u;-w ${SIZE};-h ${SIZE};-o ${ConiferAddExecutable_RES_INTERNAL_DIR}/icon.iconset/icon_${SIZE}x${SIZE}.png;${ConiferAddExecutable_ICON_SVG})
                    math(EXPR DOUBLE_SIZE "${SIZE} * 2")
                    execute_process(COMMAND ${SVG_CONVERTER} -a;-u;-w ${DOUBLE_SIZE};-h ${DOUBLE_SIZE};-o ${ConiferAddExecutable_RES_INTERNAL_DIR}/icon.iconset/icon_${SIZE}x${SIZE}@2x.png;${ConiferAddExecutable_ICON_SVG})
                endforeach ()
                execute_process(COMMAND ${ICON_UTIL} --convert icns;${ConiferAddExecutable_RES_INTERNAL_DIR}/icon.iconset)
                execute_process(COMMAND ${CMAKE_COMMAND} -E;rm;-rf;${ConiferAddExecutable_RES_INTERNAL_DIR}/logos)
                execute_process(COMMAND ${CMAKE_COMMAND} -E;rename;${ConiferAddExecutable_RES_INTERNAL_DIR}/icon.iconset;${ConiferAddExecutable_RES_INTERNAL_DIR}/logos)
                execute_process(COMMAND ${CMAKE_COMMAND} -E;rm;-rf;${ConiferAddExecutable_RES_INTERNAL_DIR}/icon.iconset)
            endif ()
            RunOnceStallAllEndBlock(${ConiferAddExecutable_BUILD_RES}/.lock CONTINUE)
        endif ()

        file(GLOB_RECURSE RESOURCES FOLLOW_SYMLINKS RELATIVE ${ConiferAddExecutable_RES_INTERNAL_DIR} CONFIGURE_DEPENDS ${ConiferAddExecutable_RES_INTERNAL_DIR}/*)
        foreach (FILE ${RESOURCES})
            cmake_path(SET DESTINATION NORMALIZE Resources/${FILE}/..)
            set_source_files_properties(${ConiferAddExecutable_RES_INTERNAL_DIR}/${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "${DESTINATION}")
            set(ConiferAddExecutable_SOURCES ${ConiferAddExecutable_SOURCES};${ConiferAddExecutable_RES_INTERNAL_DIR}/${FILE})
        endforeach ()

        file(GLOB_RECURSE RESOURCES FOLLOW_SYMLINKS RELATIVE ${ConiferAddExecutable_BUILD_RES} CONFIGURE_DEPENDS ${ConiferAddExecutable_BUILD_RES}/shaders/*)
        foreach (FILE ${RESOURCES})
            cmake_path(SET DESTINATION NORMALIZE Resources/${FILE}/..)
            set_source_files_properties(${ConiferAddExecutable_BUILD_RES}/${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "${DESTINATION}")
            set(ConiferAddExecutable_SOURCES ${ConiferAddExecutable_SOURCES};${ConiferAddExecutable_BUILD_RES}/${FILE})
        endforeach ()

        set(BUNDLE MACOSX_BUNDLE)
    endif ()

    # Windows specific code
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set(BUNDLE WIN32)
    endif ()

    add_executable(${ConiferAddExecutable_EXECUTABLE} ${BUNDLE} ${ConiferAddExecutable_SOURCES})

    # macOS specific code
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(MACOSX_BUNDLE_PACKAGE_TYPE "APPL" PARENT_SCOPE)
        set(MACOSX_BUNDLE_IDENTIFIER "com.illuminationengine.conifercomputing" PARENT_SCOPE)
        set(MACOSX_BUNDLE_NAME "${ConiferAddExecutable_NAME}" PARENT_SCOPE)
        set(MACOSX_BUNDLE_VERSION "${ConiferAddExecutable_VERSION}" PARENT_SCOPE)
        set(MACOSX_BUNDLE_COPYRIGHT "Please no stealing!" PARENT_SCOPE)
        set(MACOSX_BUNDLE_EXECUTABLE "${ConiferAddExecutable_EXECUTABLE}" PARENT_SCOPE)
        set(MACOSX_BUNDLE_ICON "icon.icns" PARENT_SCOPE)
        set_target_properties(${ConiferAddExecutable_EXECUTABLE} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${ConiferAddExecutable_BUILD_RES}/macOS/Info.plist.in)
    endif ()
endfunction()