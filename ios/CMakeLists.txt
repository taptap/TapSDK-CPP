set(MACOSX_DEPLOYMENT_TARGET 14.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(APP_SOURCES
        src/main.mm
        src/AppDelegate.h
        src/AppDelegate.mm
        src/SceneDelegate.mm
        src/SceneDelegate.h
        src/ViewController.mm
        src/ViewController.h
)

set(STORY_BOARD_FILES
        LaunchScreen.storyboard)

file(GLOB_RECURSE ICON_FILES icons/*)
set_source_files_properties(${STORY_BOARD_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

add_executable(app MACOSX_BUNDLE ${APP_SOURCES} ${STORY_BOARD_FILES} ${ICON_FILES})
target_link_libraries(app PRIVATE tds_core)
target_link_libraries(app PRIVATE "-framework UIKit")
target_link_libraries(app PRIVATE "-framework CoreGraphics")

set_target_properties(app PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_LIST_DIR}/iOS.entitlements"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${MACOSX_DEPLOYMENT_TARGET}
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "iPhone/iPad"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        XCODE_ATTRIBUTE_ENABLE_BITCODE NO
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
)