add_library("core" STATIC

src/config.h

src/types.h 
src/types.cpp

src/buffer_layout.h
src/buffer_layout.cpp

src/debug.h
src/debug.cpp

src/memory_watcher.h
src/memory_watcher.cpp

src/shared.h

src/shared_id.h
src/shared_id.cpp

src/undicht_thread.h

src/unique_object.h

)

target_include_directories("core" PUBLIC src)