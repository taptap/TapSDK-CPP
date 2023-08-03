target_link_libraries(tds_core PRIVATE
        objc
        "-framework CoreFoundation"
        "-framework Foundation")

set(TARGET_DEPENDENCIES)
get_target_property(TARGET_DEPENDENCIES tds_core LINK_LIBRARIES)
foreach (TARGET_DEPENDENCY ${TARGET_DEPENDENCIES})
    message(STATUS "TARGET_DEPENDENCY: ${TARGET_DEPENDENCY}")
endforeach ()

# CMake 直接打包 dylib 需要手动合并依赖的 targets
add_custom_target(
        sdk_combine
        COMMAND libtool -static -o $<TARGET_FILE:tds_core>
        $<TARGET_FILE:tds_core>
        $<TARGET_FILE:base>
        $<TARGET_FILE:fmt>
        $<TARGET_FILE:net>
        $<TARGET_FILE:core>
        $<TARGET_FILE:platform>
        $<TARGET_FILE:duration>
        $<TARGET_FILE:EventBus>
        $<TARGET_FILE:ssl>
        $<TARGET_FILE:crypto>
        $<TARGET_FILE:bindings-csharp>
        DEPENDS tds_core net base core platform bindings-csharp
        COMMENT "Combining libs..."
)

set_target_properties(tds_core PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER com.taptap.tapsdk
        VERSION 1.0.0
        SOVERSION 1.0.0
        PUBLIC_HEADER "${PUBLIC_HEADER_FILES}"
        )
