# Add all subdirectories
add_subdirectory(Core)
add_subdirectory(InputModule)
add_subdirectory(GraphicsModule)

# Create and define properties for the executable target
ConiferAddExecutable(IlluminationEngine ${IE_BASE_DIR}/build-res
        NAME "Illumination Engine"
        VERSION ${ILLUMINATION_ENGINE_VERSION}
        ICON_SVG ${CMAKE_SOURCE_DIR}/build-res/IlluminationEngineLogo.svg
        RES_INTERNAL_DIR ${ILLUMINATION_ENGINE_INTERNAL_RESOURCE_DIR}
        SOURCES main.cpp
        )

# Add internal dependency libraries to the target
target_link_libraries(IlluminationEngine PUBLIC IEGraphicsModule IEInputModule IECore)