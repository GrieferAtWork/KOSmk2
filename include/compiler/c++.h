/* Copyright (c) 2017 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __NO_PROTOTYPES
#undef __NO_PROTOTYPES
#undef __P
#define __P(x) x
#endif

#define __DECL_BEGIN extern "C" {
#define __DECL_END   }

#if __has_feature(cxx_constexpr) || \
   (defined(__cpp_constexpr) && __cpp_constexpr >= 200704) || \
   (defined(__IBMCPP__) && defined(__IBMCPP_CONSTEXPR) && (__IBMCPP_CONSTEXPR+0)) || \
   (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5130) || \
   (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GCC_VERSION(4,6,0) && !defined(__INTELLISENSE__)) || \
   (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026)
#   define __COMPILER_HAVE_CXX11_CONSTEXPR 1
#   define __CXX11_CONSTEXPR          constexpr
#   define __CXX11_CONSTEXPR_OR_CONST constexpr
#else
#   define __CXX11_CONSTEXPR          /* Nothing */
#   define __CXX11_CONSTEXPR_OR_CONST const
#endif

#if (defined(__clang__) && !(!__has_feature(cxx_generic_lambdas) || \
                            !(__has_feature(cxx_relaxed_constexpr) || \
                              __has_extension(cxx_relaxed_constexpr)))) || \
    (defined(__cpp_constexpr) && __cpp_constexpr >= 201304 && !defined(__clang__))
#   define __COMPILER_HAVE_CXX14_CONSTEXPR 1
#   define __CXX14_CONSTEXPR          constexpr
#   define __CXX14_CONSTEXPR_OR_CONST constexpr
#else
#   define __CXX14_CONSTEXPR          /* Nothing */
#   define __CXX14_CONSTEXPR_OR_CONST const
#endif

#if __has_feature(cxx_noexcept) || \
   (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GCC_VERSION(4,6,0) && !defined(__INTELLISENSE__)) || \
   (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190021730)
#   define __COMPILER_HAVE_CXX11_NOEXCEPT 1
#   define __CXX_NOEXCEPT noexcept
#else
#   define __CXX_NOEXCEPT throw()
#endif

#if !defined(__INTELLISENSE__) && !defined(__KERNEL__)
#define __COMPILER_PREFERR_ENUMS 1
#endif

#define __NAMESPACE_STD_BEGIN    namespace std {
#define __NAMESPACE_STD_END      }
#define __NAMESPACE_STD_SYM      ::std::
#define __NAMESPACE_STD_USING(x) using ::std::x;
#define __NAMESPACE_INT_BEGIN    namespace __int {
#define __NAMESPACE_INT_END      }
#define __NAMESPACE_INT_SYM      ::__int::
#define __NAMESPACE_INT_USING(x) using ::__int::x;
#define __BOOL                   bool

__NAMESPACE_INT_BEGIN
extern "C++" {
class __any_ptr {
    void *__m_p;
public:
    __CXX11_CONSTEXPR __ATTR_INLINE __any_ptr(void *__p) __CXX_NOEXCEPT: __m_p(__p) {}
    template<class __T> __CXX11_CONSTEXPR __ATTR_INLINE
    operator __T *(void) __CXX_NOEXCEPT { return (__T *)this->__m_p; }
}; }
__NAMESPACE_INT_END
#define __COMPILER_UNIPOINTER(p) \
        __NAMESPACE_INT_SYM __any_ptr((void *)(__UINTPTR_TYPE__)(p))
