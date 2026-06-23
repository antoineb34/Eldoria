add_executable(eldoria_cache_tests
    CacheSmokeTests.cpp
)

target_link_libraries(eldoria_cache_tests PRIVATE
    Eldoria::Data
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(eldoria_cache_tests)
