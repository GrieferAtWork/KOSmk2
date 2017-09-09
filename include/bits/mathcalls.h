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



/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional. */


/* Trigonometric functions. */
__MATHCALL(acos,,(_Mdouble_ __x)); /* Arc cosine of X. */
__MATHCALL(asin,,(_Mdouble_ __x)); /* Arc sine of X. */
__MATHCALL(atan,,(_Mdouble_ __x)); /* Arc tangent of X. */
__MATHCALL(atan2,,(_Mdouble_ __y,_Mdouble_ __x)); /* Arc tangent of Y/X. */
__MATHCALL_VEC(cos,,(_Mdouble_ __x)); /* Cosine of X. */
__MATHCALL_VEC(sin,,(_Mdouble_ __x)); /* Sine of X. */
__MATHCALL(tan,,(_Mdouble_ __x)); /* Tangent of X. */

/* Hyperbolic functions. */
__MATHCALL(cosh,,(_Mdouble_ __x)); /* Hyperbolic cosine of X. */
__MATHCALL(sinh,,(_Mdouble_ __x)); /* Hyperbolic sine of X. */
__MATHCALL(tanh,,(_Mdouble_ __x)); /* Hyperbolic tangent of X. */

#ifdef __USE_GNU
/* Cosine and sine of X. */
__MATHDECL_VEC(void,sincos,,(_Mdouble_ __x, _Mdouble_ *__sinx, _Mdouble_ *__cosx));
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_ISOC99)
__MATHCALL(acosh,,(_Mdouble_ __x)); /* Hyperbolic arc cosine of X. */
__MATHCALL(asinh,,(_Mdouble_ __x)); /* Hyperbolic arc sine of X. */
__MATHCALL(atanh,,(_Mdouble_ __x)); /* Hyperbolic arc tangent of X. */
#endif

/* Exponential and logarithmic functions. */
__MATHCALL_VEC(exp,,(_Mdouble_ __x)); /* Exponential function of X. */
__MATHCALL(frexp,,(_Mdouble_ __x,int *__exponent)); /* Break VALUE into a normalized fraction and an integral power of 2. */
__MATHCALL(ldexp,,(_Mdouble_ __x,int __exponent)); /* X times (two to the EXP power). */
__MATHCALL_VEC(log,,(_Mdouble_ __x)); /* Natural logarithm of X. */
__MATHCALL(log10,,(_Mdouble_ __x)); /* Base-ten logarithm of X. */
__NONNULL((2)) __MATHCALL(modf,,(_Mdouble_ __x,_Mdouble_ *__iptr)); /* Break VALUE into integral and fractional parts. */

#ifdef __USE_GNU
__MATHCALL(exp10,,(_Mdouble_ __x)); /* A function missing in all standards: compute exponent to base ten. */
__MATHCALL(pow10,,(_Mdouble_ __x)); /* Another name occasionally used. */
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_ISOC99)
__MATHCALL(expm1,,(_Mdouble_ __x)); /* Return exp(X) - 1. */
__MATHCALL(log1p,,(_Mdouble_ __x)); /* Return log(1 + X). */
__MATHCALL(logb,,(_Mdouble_ __x)); /* Return the base 2 signed integral exponent of X. */
#endif

#ifdef __USE_ISOC99
__MATHCALL(exp2,,(_Mdouble_ __x)); /* Compute base-2 exponential of X. */
__MATHCALL(log2,,(_Mdouble_ __x)); /* Compute base-2 logarithm of X. */
#endif

/* Power functions. */
__MATHCALL_VEC(pow,,(_Mdouble_ __x, _Mdouble_ __y)); /* Return X to the Y power. */
__MATHCALL(sqrt,,(_Mdouble_ __x)); /* Return the square root of X. */

#if defined(__USE_XOPEN) || defined(__USE_ISOC99)
__MATHCALL(hypot,,(_Mdouble_ __x, _Mdouble_ __y)); /* Return `sqrt(X*X + Y*Y)'. */
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_ISOC99)
__MATHCALL(cbrt,,(_Mdouble_ __x)); /* Return the cube root of X. */
#endif

/* Nearest integer, absolute value, and remainder functions. */
__MATHCALLX(ceil,,(_Mdouble_ __x),(__const__)); /* Smallest integral value not less than X. */
__MATHCALLX(fabs,,(_Mdouble_ __x),(__const__)); /* Absolute value of X. */
__MATHCALLX(floor,,(_Mdouble_ __x),(__const__)); /* Largest integer not greater than X. */
__MATHCALL(fmod,,(_Mdouble_ __x,_Mdouble_ __y)); /* Floating-point modulo remainder of X/Y. */
__MATHDECL_1(int,__isinf,,(_Mdouble_ __value)) __ATTR_CONST; /* Return 0 if VALUE is finite or NaN, +1 if it is +Infinity, -1 if it is -Infinity. */
__MATHDECL_1(int,__finite,,(_Mdouble_ __value)) __ATTR_CONST; /* Return nonzero if VALUE is finite and not NaN. */

#ifdef __USE_MISC
#if (!defined(__cplusplus) || __cplusplus < 201103L /* isinf conflicts with C++11. */ \
           || __MATH_DECLARING_DOUBLE == 0) /* isinff or isinfl don't. */
__MATHDECL_1(int,isinf,,(_Mdouble_ __value)) __ATTR_CONST; /* Return 0 if VALUE is finite or NaN, +1 if it is +Infinity, -1 if it is -Infinity. */
#endif
__MATHDECL_1(int,finite,,(_Mdouble_ __value)) __ATTR_CONST; /* Return nonzero if VALUE is finite and not NaN. */
__MATHCALL(drem,,(_Mdouble_ __x, _Mdouble_ __y)); /* Return the remainder of X/Y. */
__MATHCALL(significand,,(_Mdouble_ __x)); /* Return the fractional part of X after dividing out `ilogb (X)'. */
#endif /* __USE_MISC */
#ifdef __USE_ISOC99
__MATHCALLX(copysign,,(_Mdouble_ __x,_Mdouble_ __y),(__const__)); /* Return X with its signed changed to Y's. */
__MATHCALLX(nan,,(char const *__tagb),(__const__)); /* Return representation of qNaN for double type. */
#endif /* __USE_ISOC99 */

__MATHDECL_1(int,__isnan,,(_Mdouble_ __value)) __ATTR_CONST; /* Return nonzero if VALUE is not a number. */
#if defined(__USE_MISC) || (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
#if (!defined __cplusplus || __cplusplus < 201103L /* isnan conflicts with C++11. */ \
   || __MATH_DECLARING_DOUBLE == 0) /* isnanf or isnanl don't. */
__MATHDECL_1(int,isnan,,(_Mdouble_ __value)) __ATTR_CONST; /* Return nonzero if VALUE is not a number. */
#endif
#endif

#if defined(__USE_MISC) || (defined(__USE_XOPEN) && __MATH_DECLARING_DOUBLE)
/* Bessel functions. */
__MATHCALL(j0,,(_Mdouble_));
__MATHCALL(j1,,(_Mdouble_));
__MATHCALL(jn,,(int,_Mdouble_));
__MATHCALL(y0,,(_Mdouble_));
__MATHCALL(y1,,(_Mdouble_));
__MATHCALL(yn,,(int,_Mdouble_));
#endif

#if defined(__USE_XOPEN) || defined(__USE_ISOC99)
/* Error and gamma functions. */
__MATHCALL(erf,,(_Mdouble_));
__MATHCALL(erfc,,(_Mdouble_));
__MATHCALL(lgamma,,(_Mdouble_));
#endif
#ifdef __USE_ISOC99
__MATHCALL(tgamma,,(_Mdouble_)); /* True gamma function. */
#endif
#if defined(__USE_MISC) || (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__MATHCALL(gamma,,(_Mdouble_)); /* Obsolete alias for `lgamma'. */
#endif
#ifdef __USE_MISC
/* Reentrant version of lgamma. This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it. */
__MATHCALL(lgamma,_r,(_Mdouble_, int *__signgamp));
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_ISOC99)
__MATHCALL(rint,,(_Mdouble_ __x)); /* Return the integer nearest X in the direction of the prevailing rounding mode. */
__MATHCALLX(nextafter,,(_Mdouble_ __x, _Mdouble_ __y),(__const__)); /* Return X + epsilon if X < Y, X - epsilon if X > Y. */
__MATHCALL(remainder,,(_Mdouble_ __x, _Mdouble_ __y)); /* Return the remainder of integer divison X / Y with infinite precision. */
__MATHDECL(int,ilogb,,(_Mdouble_ __x)); /* Return the binary exponent of X, which must be nonzero. */
#ifdef __USE_ISOC99
__MATHCALLX(nexttoward,,(_Mdouble_ __x, long double __y),(__const__));
__MATHCALL(scalbn,,(_Mdouble_ __x, int __n)); /* Return X times (2 to the Nth power). */
#endif /* __USE_ISOC99 */
#endif /* __USE_XOPEN_EXTENDED || __USE_ISOC99 */

#ifdef __USE_ISOC99
__MATHCALL(scalbln,,(_Mdouble_ __x, long int __n)); /* Return X times (2 to the Nth power). */
__MATHCALL(nearbyint,,(_Mdouble_ __x)); /* Round X to integral value in floating-point format using current rounding direction, but do not raise inexact exception. */
__MATHCALLX(round,,(_Mdouble_ __x),(__const__)); /* Round X to nearest integral value, rounding halfway cases away from zero. */
__MATHCALLX(trunc,,(_Mdouble_ __x),(__const__)); /* Round X to the integral value in floating-point format nearest but not larger in magnitude. */
__MATHCALL(remquo,,(_Mdouble_ __x, _Mdouble_ __y, int *__quo)); /* Compute remainder of X and Y and put in *QUO a value with sign of x/y and magnitude congruent `mod 2^n' to the magnitude of the integral quotient x/y, with n >= 3. */

/* Conversion functions. */
__MATHDECL(long int,lrint,,(_Mdouble_ __x)); /* Round X to nearest integral value according to current rounding direction. */
__MATHDECL(__LONGLONG,llrint,,(_Mdouble_ __x));
__MATHDECL(long int,lround,,(_Mdouble_ __x)); /* Round X to nearest integral value, rounding halfway cases away from zero. */
__MATHDECL(__LONGLONG,llround,,(_Mdouble_ __x));
__MATHCALL(fdim,,(_Mdouble_ __x, _Mdouble_ __y)); /* Return positive difference between X and Y. */
__MATHCALLX(fmax,,(_Mdouble_ __x, _Mdouble_ __y),(__const__)); /* Return maximum numeric value from X and Y. */
__MATHCALLX(fmin,,(_Mdouble_ __x, _Mdouble_ __y),(__const__)); /* Return minimum numeric value from X and Y. */
__MATHDECL_1(int,__fpclassify,,(_Mdouble_ __value)) __ATTR_CONST; /* Classify given number. */
__MATHDECL_1(int,__signbit,,(_Mdouble_ __value)) __ATTR_CONST; /* Test for negative number. */
__MATHCALL(fma,,(_Mdouble_ __x,_Mdouble_ __y,_Mdouble_ __z)); /* Multiply-add function computed as a ternary operation. */
#endif /* __USE_ISOC99 */
#ifdef __USE_GNU
__MATHDECL_1(int,__issignaling,,(_Mdouble_ __value)) __ATTR_CONST;
#endif /* __USE_GNU */
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && __MATH_DECLARING_DOUBLE && !defined __USE_XOPEN2K8)
__MATHCALL(scalb,,(_Mdouble_ __x, _Mdouble_ __n)); /* Return X times (2 to the Nth power). */
#endif
