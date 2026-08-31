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
# "Freestanding by construction" must actually be constructed: the top-level
# include_directories() would leak src/Core and src/server into this target,
# letting a Core/transport include compile clean. Pin the target's own
# include path so the compiler enforces the boundary, not just review.
set_target_properties(de-core PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/src")
