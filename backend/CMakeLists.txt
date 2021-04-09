# abstract operation-system layer
    
cmake_minimum_required(VERSION 3.10)

set(BASE_DIR "/home/veit/git_GLOBEON/globeon/")

INCLUDE_DIRECTORIES("${BASE_DIR}/qbus_collection/libs/cape/src" "${BASE_DIR}/m_qore/l_gen/src" "${BASE_DIR}/qbus_collection/libs/qbus/src" "${BASE_DIR}/qbus_collection/libs/qwebs/src")

link_directories("${BASE_DIR}/build/qbus_collection/libs/cape/src/")
link_directories("${BASE_DIR}/build/qbus_collection/libs/qbus/src/")
link_directories("${BASE_DIR}/build/qbus_collection/libs/qwebs/src/")

set(SRV_SOURCES
    webserver.cpp
    entity.cpp
)

set(SRV_HEADERS
)

add_executable          (webserver ${SRV_SOURCES})
target_link_libraries   (webserver cape qbus qwebs)
