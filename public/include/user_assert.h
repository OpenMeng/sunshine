/**
 * @file user_assert.h
 * @brief Assert - abort the program if assertion is false.
 *
 * If the macro DEBUG was defined, we'll use the C Library's assert(). Otherwise,
 * we'll define the macro assert(), it generates no code, and hence does nothing
 * at all.
 * 
 */

#ifndef __USER_ASSERT_H__
#define __USER_ASSERT_H__

// Remove assert in normal mode.
#if !defined(DEBUG)
#undef assert
#define assert(stmt)
#else
#include <cassert>
#endif

#endif

