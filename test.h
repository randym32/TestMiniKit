/** Test Mini-Kit
    Copyright 2018, Randall Maas
 
    The test asserts are:
      TstFail               -- Generates a failure immediately and unconditionally.
      TstTrue               -- Asserts that an expression is true.
      TstFalse              -- Asserts that an expression is true.
      TstNull               -- Asserts that an expression is nil.
      TstNotNull            -- Asserts that an expression is not nil.
      TstEquals             -- Asserts that two expressions have the same value (with tolerance for floating points) or equal objects
      TstEquivalent         -- Asserts that two expressions have the exact same value
      TstNotEquals          -- Asserts that two expressions do not have the same value.
      TstNotEquivalent      -- Asserts that two expressions do not have the exact same value
      TstGreaterThan        -- Asserts that the value of one expression is greater than another.
      TstGreaterThanOrEqual -- Asserts that the value of one expression is greater than or equal to another.
      TstLessThanOrEqual    -- Asserts that the value of one expression is less than or equal to another.
      TstLessThan           -- Asserts that the value of one expression is less than another.
 
    You will need to enable the compiler to use the C++14 version of the C++ language to use most of these.
 
    Other credits
     Portions were inspired by
     https://news.ycombinator.com/item?id=17782884#17783369
     XCTest
*/
#include <stdio.h>

// Most of this is junk to handle the weirdo compiler behaviours

// handle strangeness with MSVC and Clang
#define _MSVC_COMBINE(x,y)  x ## y
#define MSVC_COMBINE(x,y)  _MSVC_COMBINE(x,y)


// Clang on Windows or OS X or..
#if defined(__clang__)
#include <inttypes.h>
// This is needed to deal with handling of __VA_ARGS__ expansion
#define _hack2(a,b,...) b
#define testCase(...)   _testCase(_hack2(_, ##__VA_ARGS__, MSVC_COMBINE(test,__LINE__)))
#endif

#if defined(_MSC_VER)

#if _MSC_VER < 1600
#define PRId64 "lld"
#define PRIx64 "llx"
#else
#include <inttypes.h>
#endif

#ifndef __clang__
// This is needed to deal with MSVC's unexpected handling of __VA_ARGS__ expansion
#define _ex(_) _
#define _hack3(a,b,...) b
#define _hack2(a,...)   _ex(_hack3(__VA_ARGS__))
#define _hack1(...)     _hack2(_, _, __VA_ARGS__, MSVC_COMBINE(test,__LINE__))
#define testCase(...)   _testCase(_hack1(__VA_ARGS__))
#endif

/// OSX
#elif defined(__clang__)
#include <string.h>

#define _testCase(f,...) \
        static void f(void); \
        static void (*MSVC_COMBINE(f,_))(void) __attribute__ ((used,section ("__DATA,__rcmTst"))) = f; \
        static void f(void)
// This is needed to deal with handling of __VA_ARGS__ expansion
#define _hack2(a,b,...) b
#define testCase(...)   _testCase(_hack2(_, ##__VA_ARGS__, MSVC_COMBINE(test,__LINE__)))
extern "C" int AmIBeingDebugged(void);
#define BREAKPOINT() do{ if(AmIBeingDebugged()) {__asm__("int $3\n" : : );}} while(false)
#endif

#ifdef _MSC_VER
#include <windows.h>
#include <winbase.h>
#ifndef BREAKPOINT
#define BREAKPOINT() __debugbreak()
#endif

#pragma section(".rcmTst$b",read)
#if defined(__clang__)
#define _testCase(f,...) \
        static void f(void); \
        static void (*MSVC_COMBINE(f,_))(void) __attribute__ ((used,section (".rcmTst$b"))) = f; \
        static void f(void)
#else
#define _testCase(f,...) \
        static void f(void); \
        static __declspec(allocate(".rcmTst$b")) void (*MSVC_COMBINE(f,_))(void) = f; \
        static void f(void)
#endif

#define _ErrMsgHdr   "%s(%" PRId64 "):%s:"
//#define _ErrPrint(...) do{char buf[512]; snprintf(buf, sizeof(buf), __VA_ARGS__); OutputDebugStringA(buf);}while(0)
#else
#include <inttypes.h>
#define _ErrMsgHdr   "%s:%" PRId64  ":%s:"
#endif
#define _ErrPrint(...) fprintf(stderr,  __VA_ARGS__);


/// helpers to print values
inline void _errorValue(int        a) {_ErrPrint("%d (0x%x)\n", a,a);}
inline void _errorValue(unsigned   a) {_ErrPrint("%u (0x%x)\n", a,a);}
inline void _errorValue(int64_t    a) {_ErrPrint("%" PRId64 " (0x%" PRIx64 ")\n", a,a);}
inline void _errorValue(double     a) {_ErrPrint("%g  (%f)\n", a,a);}
inline void _errorValue(char       a) {_ErrPrint("'%c' (0x%x, %d)\n", a, (unsigned)a, (unsigned)a);}
inline void _errorValue(char const*a) {_ErrPrint("\"%s\"\n", a);}
inline void _errorValue(void const*a) {_ErrPrint("0x%p\n", a);}

template <typename t> inline auto isEqual(t a, t b) { return a == b; }
inline auto isEqual(char const* a, char const* b)   { return 0 == strcmp(a, b);}
inline auto isEqual(double a, double b)
{
   double c = a - b; c *= c;
   // Change this threshold to whatever is most interesting
   return c < 0.0000001;
}

/// A helper to report on the test results
template <typename v1>
void _unitReport(v1 a, char const* errMsg, const char *a_str,const char *b_str,const char *file,int64_t line, char const* funcName, char const *message )
{
   // First, print which test is being performed (ie, the function, file and
   // line number) and report that we failed , and any caller supplied error message
   _ErrPrint(_ErrMsgHdr "   test failed: %s\n", file, line, funcName
               ,message && message[0]? message:errMsg);
   if (!message || !*message) _ErrPrint("   %s\n", errMsg);

   // Display the expressions that were used
   _ErrPrint("    Got    expr : %s\n",a_str);
   _ErrPrint("    Wanted expr : %s\n",b_str);
   // Display the value produced by the expression and expected
   _ErrPrint("    Got    value: "); _errorValue(a);
}

/// A helper to perform a test and report the results
template <typename n, typename v1, typename v2>
int _unitCheck(n fn, v1 a,v2 b, char const* errMsg, const char *a_str,const char *b_str,const char *file,int64_t line, char const* funcName, char const *message )
{
   // Next, perform the test and say whether or not we passed the test
   if(fn(a,b))
   {
      return 0;
   }

   // Display the error message
   _unitReport(a, errMsg, a_str, b_str, file, line, funcName, message);
   _ErrPrint("    Wanted value: "); _errorValue(b);
   return 1;
}


/// A helper to perform a test and report the results
template <typename n, typename v1>
int _unitCheck(n fn, v1 a, char const* errMsg, const char *a_str,const char *b_str,const char *file,int64_t line,char const* funcName, char const * message)
{
   // Next, perform the test and say whether or not we passed the test
   if(fn(a))
   {
      return 0;
   }

   // Display the error message
   _unitReport(a, errMsg, a_str, b_str, file, line, funcName, message);
   return 1;
}

// Throw a breakpoint to put developer into the right location to debug it.
// Note:  The BREAKPOINT() invocation is here so that the debugger in the right
// location.   Place in the above procedures will have the debugger at the
// wrong code location.
#define _unitCheck1(FN,A,ExpValue,Msg1,Msg2) do{if(_unitCheck([](auto const a             ){return FN;}, (A),    Msg1,#A,ExpValue,__FILE__,__LINE__,__FUNCTION__, Msg2))BREAKPOINT();}while(0)
#define _unitCheck2(FN,A,B,Msg1,Msg2)        do{if(_unitCheck([](auto const a,auto const b){return FN;}, (A),(B),Msg1,#A,#B      ,__FILE__,__LINE__,__FUNCTION__, Msg2))BREAKPOINT();}while(0)

#define TstFail(...)                   do{_ErrPrint(_ErrMsgHdr "   test failed: %s\n",__FILE__,(uint64_t) __LINE__,__FUNCTION__,__VA_ARGS__ ""); BREAKPOINT();}while(0)

#define TstTrue(A,...)                 _unitCheck1(        a    ,A,  "Value not true." ,"true"                          ,__VA_ARGS__ "")
#define TstFalse(A,...)                _unitCheck1(       !a    ,A,  "Value not false.","false"                         ,__VA_ARGS__ "")
#define TstNull(A,...)                 _unitCheck1( NULL == a   ,((void const*)A),  "Value not null." ,"not null"       ,__VA_ARGS__ "")
#define TstNotNull(A,...)              _unitCheck1( NULL != a   ,((void const*)A),  "Value null."     ,"not null"       ,__VA_ARGS__ "")
#define TstEquivalent(A,B,...)         _unitCheck2(  a == b     ,A,B,"Values are not equivalent."                       ,__VA_ARGS__ "")
#define TstNotEquivalent(A,B,...)      _unitCheck2(  a != b     ,A,B,"Values are equivalent."                           ,__VA_ARGS__ "")
#define TstNotEquals(A,B,...)          _unitCheck2(!isEqual(a,b),A,B,"Values are equal."                                ,__VA_ARGS__ "")
#define TstEquals(A,B,...)             _unitCheck2( isEqual(a,b),A,B,"Values are not equal."                            ,__VA_ARGS__ "")
#define TstLessThan(A,B,...)           _unitCheck2(     a< b    ,A,B,"Value is not less than the second."               ,__VA_ARGS__ "")
#define TstLessThanOrEqual(A,B,...)    _unitCheck2(     a<=b    ,A,B,"Value is not less than or equal to the second."   ,__VA_ARGS__ "")
#define TstGreaterThan(A,B,...)        _unitCheck2(     a >b    ,A,B,"Value is not greater than the second."            ,__VA_ARGS__ "")
#define TstGreaterThanOrEqual(A,B,...) _unitCheck2(     a>=b    ,A,B,"Value is not greater than or equal to the second.",__VA_ARGS__ "")

/// Run the tests
extern void testsRun();
