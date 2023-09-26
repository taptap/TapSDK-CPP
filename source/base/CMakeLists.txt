add_library(tapsdk_base STATIC
        common_funcs.h
        types.h
        stack_container.h
        alignment.h
        blocking_queue.h
        scoped_generic.h
        expected.h
        logging.h
        logging.cpp
        event.h
        timer.cpp
        timer.h
        thread.cpp
        thread.h
        spin_lock.h
        spin_lock.cpp
        expected.h
        uuid.cpp
        uuid.h
        md5.cpp
        md5.h
        base64.cpp
        base64.h
        hmac.h
        sha1.cpp
        sha1.h
        file.cpp
        file.h
)

target_link_libraries(tapsdk_base PUBLIC fmt EventBus ghc_filesystem)

if (ANDROID)
    target_link_libraries(tapsdk_base PUBLIC log)
endif ()

add_library(tapsdk::base ALIAS tapsdk_base)