//--------------------------------------------------------------------------------
//
// Filename   : Assert.h
// Written By : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __ASSERT_H__
#define __ASSERT_H__

// include files
#include <source_location>

#include "Exception.h"
#include "Types.h"

//--------------------------------------------------------------------------------
//
// Call-site diagnostics without location macros.
//
// The call site stays a macro because the diagnostic needs the unevaluated
// text of the expression (#expr) and the expression must be evaluated exactly
// once. Everything else - file, line and enclosing function - is captured by
// the defaulted std::source_location parameter, so no __FILE__ / __LINE__ /
// __PRETTY_FUNCTION__ plumbing is left here. Under Clang
// std::source_location::function_name() yields the same text
// __PRETTY_FUNCTION__ produced, so the logged message is unchanged.
//
// A failing Assert appends to assertion_failed.log in the working directory
// and throws AssertionError.
//
//--------------------------------------------------------------------------------
[[noreturn]] void assertionFailed(const char* expr,
                                  const std::source_location& loc = std::source_location::current()) noexcept(false);

//--------------------------------------------------------------------------------
//
// ProtocolAssert lets the server react when a hacked or broken client sends
// invalid data: it appends to protocol_assertion_failed.log and throws
// InvalidProtocolException, which the packet layer turns into a disconnect.
//
//--------------------------------------------------------------------------------
[[noreturn]] void
protocolAssertionFailed(const char* expr,
                        const std::source_location& loc = std::source_location::current()) noexcept(false);

#if defined(NDEBUG)
// Still evaluate the expression: many call sites do real work inside Assert(),
// e.g. Assert(pTree->GetAttribute("class", iClass)), and dropping it silently
// breaks them. Only the diagnostic is disabled here. No build type defines
// NDEBUG - see the note in the top-level CMakeLists.txt.
#define Assert(expr) ((void)(expr))
#define ProtocolAssert(expr) ((void)(expr))
#else
// std::source_location is portable, so the former __LINUX__ / __APPLE__ /
// __WIN_CONSOLE__ / __WIN32__ / __MFC__ ladder is gone: only __LINUX__ and
// __APPLE__ were ever defined by this build, and the remaining branches
// referenced a Windows/MFC port that no longer exists.
#define Assert(expr) ((void)((expr) ? 0 : (assertionFailed(#expr), 0)))
#define ProtocolAssert(expr) ((void)((expr) ? 0 : (protocolAssertionFailed(#expr), 0)))
#endif

#endif
