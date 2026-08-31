# ============================================================================
# de-core — pure domain logic (docs/RESTRUCTURING.md Phase 3).
#
# Freestanding by construction: no Core, no server targets, no MySQL/Lua/
# transport, no server-type macros. Consumers include it as
# "domain/<Header>.h" via the PUBLIC include dir below, and the library is
# unit-tested in isolation by formula_tests (tests/CMakeLists.txt).
# ============================================================================

add_library(de-core STATIC
    Formulas.cpp
)

target_include_directories(de-core PUBLIC ${CMAKE_SOURCE_DIR}/src)
