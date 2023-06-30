add_library(base STATIC
        common_funcs.h
        types.h
        stack_container.h
        alignment.h
        blocking_queue.h
        scoped_generic.h
        expected.h
        logging.h
        logging.cpp)

target_link_libraries(base PUBLIC fmt)

if (ANDROID)
    target_link_libraries(base PUBLIC log)
endif ()