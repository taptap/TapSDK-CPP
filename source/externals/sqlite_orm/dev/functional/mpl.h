#pragma once

/*
 *  Symbols for 'template metaprogramming' (compile-time template programming),
 *  inspired by the MPL of Aleksey Gurtovoy and David Abrahams.
 *  
 *  Currently, the focus is on facilitating advanced type filtering,
 *  such as filtering columns by constraints having various traits.
 *  Hence it contains only a very small subset of a full MPL.
 *  
 *  Two key concepts are critical to understanding:
 *  1. A 'metafunction' is a class template that represents a function invocable at compile-time.
 *  2. A 'metafunction class' is a certain form of metafunction representation that enables higher-order metaprogramming.
 *     More precisely, it's a class with a nested metafunction called "fn"
 *     Correspondingly, a metafunction class invocation is defined as invocation of its nested "fn" metafunction.
 *  3. A 'metafunction operation' is an alias template that represents a function whose instantiation already yields a type.
 *
 *  Conventions:
 *  - "Fn" is the name for a metafunction template template parameter.
 *  - "FnCls" is the name for a metafunction class template parameter.
 *  - "_fn" is a suffix for a type that accepts metafunctions and turns them into metafunction classes.
 *  - "higher order" denotes a metafunction that operates on another metafunction (i.e. takes it as an argument).
 */

#include <type_traits>  //  std::false_type, std::true_type

#include "cxx_universal.h"
#include "cxx_type_traits_polyfill.h"

namespace sqlite_orm {
    namespace internal {
        namespace mpl {
            template<template<class...> class Fn>
            struct indirectly_test_metafunction;

            /*
             *  Determines whether a class template has a nested metafunction `fn`.
             * 
             *  Implementation note: the technique of specialiazing on the inline variable must come first because
             *  of older compilers having problems with the detection of dependent templates [SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION].
             */
            template<class T, class SFINAE = void>
            SQLITE_ORM_INLINE_VAR constexpr bool is_metafunction_class_v = false;
            template<class FnCls>
            SQLITE_ORM_INLINE_VAR constexpr bool
                is_metafunction_class_v<FnCls, polyfill::void_t<indirectly_test_metafunction<FnCls::template fn>>> =
                    true;

            template<class T>
            struct is_metafunction_class : polyfill::bool_constant<is_metafunction_class_v<T>> {};

            /*
             *  Invoke metafunction.
             */
            template<template<class...> class Fn, class... Args>
            using invoke_fn_t = typename Fn<Args...>::type;

#ifdef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
            template<template<class...> class Op, class... Args>
            struct wrap_op {
                using type = Op<Args...>;
            };

            /*
             *  Invoke metafunction operation.
             *  
             *  Note: legacy compilers need an extra layer of indirection, otherwise type replacement may fail
             *  if alias template `Op` has a dependent expression in it.
             */
            template<template<class...> class Op, class... Args>
            using invoke_op_t = typename wrap_op<Op, Args...>::type;
#else
            /*
             *  Invoke metafunction operation.
             */
            template<template<class...> class Op, class... Args>
            using invoke_op_t = Op<Args...>;
#endif

            /*
             *  Invoke metafunction class by invoking its nested metafunction.
             */
            template<class FnCls, class... Args>
            using invoke_t = typename FnCls::template fn<Args...>::type;

            /*
             *  Instantiate metafunction class' nested metafunction.
             */
            template<class FnCls, class... Args>
            using instantiate = typename FnCls::template fn<Args...>;

            /*
             *  Wrap given type such that `typename T::type` is valid.
             */
            template<class T, class SFINAE = void>
            struct type_wrap : polyfill::type_identity<T> {};
            template<class T>
            struct type_wrap<T, polyfill::void_t<typename T::type>> : T {};

            /*
             *  Turn metafunction into a metafunction class.
             *  
             *  Invocation of the nested metafunction `fn` is SFINAE-friendly (detection idiom).
             *  This is necessary because `fn` is a proxy to the originally quoted metafunction,
             *  and the instantiation of the metafunction might be an invalid expression.
             */
            template<template<class...> class Fn>
            struct quote_fn {
                template<class InvocableTest, template<class...> class, class...>
                struct invoke_fn;

                template<template<class...> class F, class... Args>
                struct invoke_fn<polyfill::void_t<F<Args...>>, F, Args...> {
                    using type = type_wrap<F<Args...>>;
                };

                template<class... Args>
                using fn = typename invoke_fn<void, Fn, Args...>::type;
            };

            /*
             *  Indirection wrapper for higher-order metafunctions,
             *  specialized on the argument indexes where metafunctions appear.
             */
            template<size_t...>
            struct higherorder;

            template<>
            struct higherorder<0u> {
                /*
                 *  Turn higher-order metafunction into a metafunction class.
                 */
                template<template<template<class...> class Fn, class... Args2> class HigherFn>
                struct quote_fn {
                    template<class QuotedFn, class... Args2>
                    struct fn : HigherFn<QuotedFn::template fn, Args2...> {};
                };
            };

            /*
             *  Metafunction class that extracts the nested metafunction of its metafunction class argument,
             *  quotes the extracted metafunction and passes it on to the next metafunction class
             *  (kind of the inverse of quoting).
             */
            template<class FnCls>
            struct pass_extracted_fn_to {
                template<class... Args>
                struct fn : FnCls::template fn<Args...> {};

                // extract, quote, pass on
                template<template<class...> class Fn, class... Args>
                struct fn<Fn<Args...>> : FnCls::template fn<quote_fn<Fn>> {};
            };

            /*
             *  Metafunction class that invokes the specified metafunction operation,
             *  and passes its result on to the next metafunction class.
             */
            template<template<class...> class Op, class FnCls>
            struct pass_result_to {
                // call Op, pass on its result
                template<class... Args>
                struct fn : FnCls::template fn<Op<Args...>> {};
            };

            /*
             *  Bind arguments at the front of a metafunction class.
             *  Metafunction class equivalent to std::bind_front().
             */
            template<class FnCls, class... Bound>
            struct bind_front {
                template<class... Args>
                struct fn : FnCls::template fn<Bound..., Args...> {};
            };

            /*
             *  Bind arguments at the back of a metafunction class.
             *  Metafunction class equivalent to std::bind_back()
             */
            template<class FnCls, class... Bound>
            struct bind_back {
                template<class... Args>
                struct fn : FnCls::template fn<Args..., Bound...> {};
            };

            /*
             *  Metafunction class equivalent to polyfill::always_false.
             *  It ignores arguments passed to the metafunction,
             *  and always returns the given type.
             */
            template<class T>
            struct always {
                template<class...>
                struct fn : type_wrap<T> {};
            };

            /*
             *  Unary metafunction class equivalent to std::type_identity.
             */
            struct identity {
                template<class T>
                struct fn : type_wrap<T> {};
            };

            /*
             *  Metafunction class equivalent to std::negation.
             */
            template<class FnCls>
            struct not_ {
                template<class... Args>
                struct fn : polyfill::negation<invoke_t<FnCls, Args...>> {};
            };

            /*
             *  Metafunction class equivalent to std::conjunction
             */
            template<class... TraitFnCls>
            struct conjunction {
                template<class... Args>
                struct fn : polyfill::conjunction<typename TraitFnCls::template fn<Args...>...> {};
            };

            /*
             *  Metafunction class equivalent to std::disjunction.
             */
            template<class... TraitFnCls>
            struct disjunction {
                template<class... Args>
                struct fn : polyfill::disjunction<typename TraitFnCls::template fn<Args...>...> {};
            };

#ifndef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
            /*
             *  Metafunction equivalent to std::conjunction.
             */
            template<template<class...> class... TraitFn>
            using conjunction_fn = conjunction<quote_fn<TraitFn>...>;

            /*
             *  Metafunction equivalent to std::disjunction.
             */
            template<template<class...> class... TraitFn>
            using disjunction_fn = disjunction<quote_fn<TraitFn>...>;
#else
            template<template<class...> class... TraitFn>
            struct conjunction_fn : conjunction<quote_fn<TraitFn>...> {};

            template<template<class...> class... TraitFn>
            struct disjunction_fn : disjunction<quote_fn<TraitFn>...> {};
#endif

            /*
             *  Convenience template alias for binding arguments at the front of a metafunction.
             */
            template<template<class...> class Fn, class... Bound>
            using bind_front_fn = bind_front<quote_fn<Fn>, Bound...>;

            /*
             *  Convenience template alias for binding arguments at the back of a metafunction.
             */
            template<template<class...> class Fn, class... Bound>
            using bind_back_fn = bind_back<quote_fn<Fn>, Bound...>;

            /*
             *  Convenience template alias for binding a metafunction at the front of a higher-order metafunction.
             */
            template<template<template<class...> class Fn, class... Args2> class HigherFn,
                     template<class...>
                     class BoundFn,
                     class... Bound>
            using bind_front_higherorder_fn =
                bind_front<higherorder<0>::quote_fn<HigherFn>, quote_fn<BoundFn>, Bound...>;
        }
    }

    namespace mpl = internal::mpl;

    // convenience metafunction classes
    namespace internal {
        /*
         *  Trait metafunction class that checks if a type has the specified trait.
         */
        template<template<class...> class TraitFn>
        using check_if = mpl::quote_fn<TraitFn>;

        /*
         *  Trait metafunction class that checks if a type doesn't have the specified trait.
         */
        template<template<class...> class TraitFn>
        using check_if_not = mpl::not_<mpl::quote_fn<TraitFn>>;

        /*
         *  Trait metafunction class that checks if a type is the same as the specified type.
         */
        template<class Type>
        using check_if_is_type = mpl::bind_front_fn<std::is_same, Type>;

        /*
         *  Trait metafunction class that checks if a type's template matches the specified template
         *  (similar to `is_specialization_of`).
         */
        template<template<class...> class Template>
        using check_if_is_template =
            mpl::pass_extracted_fn_to<mpl::bind_front_fn<std::is_same, mpl::quote_fn<Template>>>;
    }
}
