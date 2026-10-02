add_executable(eldoria_dataset_builder
    main.cpp
)

target_link_libraries(eldoria_dataset_builder
    PRIVATE
        Eldoria::Assets
)
