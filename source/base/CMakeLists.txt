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
)

target_link_libraries(tapsdk_base PUBLIC fmt EventBus)

if (ANDROID)
    target_link_libraries(tapsdk_base PUBLIC log)
endif ()

add_library(tapsdk::base ALIAS tapsdk_base)