if (DESKTOP_APP_CUSTOM_BACKEND)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WebSockets REQUIRED)

    add_library(custom_network STATIC)
    init_target(custom_network)
    target_sources(custom_network PRIVATE
        custom_api.cpp
        rest_client.cpp
        websocket_client.cpp
        session_manager.cpp
        data_adapter.cpp
        notification_handler.cpp
        file_transfer.cpp
        custom_auth.cpp
        karagram_backend.cpp
    )
    target_include_directories(custom_network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(custom_network PUBLIC
        desktop-app::lib_base
        desktop-app::lib_crl
        desktop-app::lib_ui
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
    )
    set_target_properties(custom_network PROPERTIES AUTOMOC ON)
    target_compile_definitions(custom_network PUBLIC CUSTOM_BACKEND)
endif()
