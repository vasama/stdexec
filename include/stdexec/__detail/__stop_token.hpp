/*
 * Copyright (c) 2021-2022 Facebook, Inc. and its affiliates
 * Copyright (c) 2021-2024 NVIDIA Corporation
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "__execution_fwd.hpp"

#include "__concepts.hpp"

namespace stdexec {
  namespace __stok {
    template <template <class> class>
    struct __check_type_alias_exists;
  } // namespace __stok

  template <class _Token, class _Callback>
  using stop_callback_for_t = typename _Token::template callback_type<_Callback>;

  template <class _Token>
  concept stoppable_token =
    __nothrow_copy_constructible<_Token>    //
    && __nothrow_move_constructible<_Token> //
    && equality_comparable<_Token>          //
    && requires(const _Token& __token) {
         { __token.stop_requested() } noexcept -> __boolean_testable_;
         { __token.stop_possible() } noexcept -> __boolean_testable_;
    // workaround ICE in appleclang 13.1
#if !defined(__clang__)
         typename __stok::__check_type_alias_exists<_Token::template callback_type>;
#endif
       };

  template <class _Token, typename _Callback, typename _Initializer = _Callback>
  concept stoppable_token_for =
    stoppable_token<_Token>  //
    && __callable<_Callback> //
    && requires { typename stop_callback_for_t<_Token, _Callback>; }
    && constructible_from<_Callback, _Initializer>
    && constructible_from<stop_callback_for_t<_Token, _Callback>, const _Token&, _Initializer>;

  template <class _Token>
  concept unstoppable_token =  //
    stoppable_token<_Token> && //
    requires {
      { _Token::stop_possible() } -> __boolean_testable_;
    } && (!_Token::stop_possible());

  template <class _Token, class _Stoppable>
  class __stoppable_base_impl {
    class __cb {
     public:
      explicit __cb(__stoppable_base_impl& __base)
        : __base_(__base) {}
      void operator()() {
        static_cast<_Stoppable&>(__base_).on_stop_requested();
      }
     private:
      __stoppable_base_impl& __base_;
    };
   protected:
    explicit __stoppable_base_impl(const _Token& __token)
      : __cb_(__token) {}
   private:
    stop_callback_for_t<_Token, __cb> __cb_;
  };

  template <class _Token>
  struct __stoppable_base_for {
    template <class _Stoppable>
    using __f = __stoppable_base_impl<_Token, _Stoppable>;
  };

  template <class _Token, class _Stoppable>
  using __stoppable_base_for_t = typename __stoppable_base_for<_Token>::template __f<_Stoppable>;

} // namespace stdexec
