add_executable(elclient
    main.cpp

    ElClientApp.cpp
    ScreenManager.cpp
    PlaceholderScreen.cpp
    LoginScreen.cpp
    InputManager.cpp
    ClientRenderContext.cpp
    UIManager.cpp
    UIContext.cpp
    PlaceholderWidget.cpp
    TextInputWidget.cpp
    ButtonWidget.cpp
    LabelWidget.cpp
)

target_include_directories(elclient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elclient PRIVATE
    Eldoria::Data
    Eldoria::Game
    Eldoria::Network
    Eldoria::Platform
    Eldoria::Render
    Eldoria::World
)

set_target_properties(elclient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)