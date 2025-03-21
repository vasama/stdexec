/* Copyright (c) 2023 Maikel Nadolski
 * Copyright (c) 2023 NVIDIA Corporation
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

#include <stdexec/execution.hpp>

#include <cstddef>

namespace exec {
  namespace __any {
    using namespace stdexec;

    struct __create_vtable_t {
      template <class _VTable, class _Tp>
        requires __tag_invocable_r<const _VTable*, __create_vtable_t, __mtype<_VTable>, __mtype<_Tp>>
      constexpr const _VTable* operator()(__mtype<_VTable>, __mtype<_Tp>) const noexcept {
        return tag_invoke(__create_vtable_t{}, __mtype<_VTable>{}, __mtype<_Tp>{});
      }
    };

    inline constexpr __create_vtable_t __create_vtable{};

    template <class _Sig>
    struct __query_vfun;

    template <class _Tag, class _Ret, class... _As>
    struct __query_vfun<_Tag (*const)(_Ret (*)(_As...))> {
      _Ret (*__fn_)(void*, _As...);

      _Ret operator()(_Tag, void* __rcvr, _As&&... __as) const {
        return __fn_(__rcvr, (_As&&) __as...);
      }
    };

    template <class _Tag, class _Ret, class... _As>
    struct __query_vfun<_Tag (*)(_Ret (*)(_As...))> {
      _Ret (*__fn_)(void*, _As...);

      _Ret operator()(_Tag, void* __rcvr, _As&&... __as) const {
        return __fn_(__rcvr, (_As&&) __as...);
      }
    };

    template <class _Tag, class _Ret, class... _As>
    struct __query_vfun<_Tag (*const)(_Ret (*)(_As...) noexcept)> {
      _Ret (*__fn_)(void*, _As...) noexcept;

      _Ret operator()(_Tag, void* __rcvr, _As&&... __as) const noexcept {
        return __fn_(__rcvr, (_As&&) __as...);
      }
    };

    template <class _Tag, class _Ret, class... _As>
    struct __query_vfun<_Tag (*)(_Ret (*)(_As...) noexcept)> {
      _Ret (*__fn_)(void*, _As...) noexcept;

      _Ret operator()(_Tag, void* __rcvr, _As&&... __as) const noexcept {
        return __fn_(__rcvr, (_As&&) __as...);
      }
    };

    template <class>
    struct __query_vfun_fn;

    template <class _EnvProvider>
      requires __callable<get_env_t, const _EnvProvider&>
    struct __query_vfun_fn<_EnvProvider> {
      template <class _Tag, class _Ret, class... _As>
        requires __callable<_Tag, env_of_t<const _EnvProvider&>, _As...>
      constexpr _Ret (*operator()(_Tag (*)(_Ret (*)(_As...))) const noexcept)(void*, _As...) {
        return +[](void* __env_provider, _As... __as) -> _Ret {
          return _Tag{}(get_env(*(const _EnvProvider*) __env_provider), (_As&&) __as...);
        };
      }

      template <class _Tag, class _Ret, class... _As>
        requires __callable<_Tag, env_of_t<const _EnvProvider&>, _As...>
      constexpr _Ret (
        *operator()(_Tag (*)(_Ret (*)(_As...) noexcept)) const noexcept)(void*, _As...) noexcept {
        return +[](void* __env_provider, _As... __as) noexcept -> _Ret {
          static_assert(__nothrow_callable<_Tag, const env_of_t<_EnvProvider>&, _As...>);
          return _Tag{}(get_env(*(const _EnvProvider*) __env_provider), (_As&&) __as...);
        };
      }
    };

    template <class _Queryable>
      requires(!__callable<get_env_t, const _Queryable&>)
    struct __query_vfun_fn<_Queryable> {
      template <class _Tag, class _Ret, class... _As>
        requires __callable<_Tag, const _Queryable&, _As...>
      constexpr _Ret (*operator()(_Tag (*)(_Ret (*)(_As...))) const noexcept)(void*, _As...) {
        return +[](void* __queryable, _As... __as) -> _Ret {
          return _Tag{}(*(const _Queryable*) __queryable, (_As&&) __as...);
        };
      }

      template <class _Tag, class _Ret, class... _As>
        requires __callable<_Tag, const _Queryable&, _As...>
      constexpr _Ret (
        *operator()(_Tag (*)(_Ret (*)(_As...) noexcept)) const noexcept)(void*, _As...) noexcept {
        return +[](void* __env_provider, _As... __as) noexcept -> _Ret {
          static_assert(__nothrow_callable<_Tag, const _Queryable&, _As...>);
          return _Tag{}(*(const _Queryable*) __env_provider, (_As&&) __as...);
        };
      }
    };

    template <class _Sig>
    struct __storage_vfun;

    template <class _Tag, class... _As>
    struct __storage_vfun<_Tag(void (*)(_As...))> {
      void (*__fn_)(void*, _As...) = [](void*, _As...) {
      };

      void operator()(_Tag, void* __storage, _As&&... __as) const {
        return __fn_(__storage, (_As&&) __as...);
      }
    };

    template <class _Tag, class... _As>
    struct __storage_vfun<_Tag(void (*)(_As...) noexcept)> {
      void (*__fn_)(void*, _As...) noexcept = [](void*, _As...) noexcept {
      };

      void operator()(_Tag, void* __storage, _As&&... __as) const noexcept {
        return __fn_(__storage, (_As&&) __as...);
      }
    };

    template <class _Storage, class _Tp>
    struct __storage_vfun_fn {
      template <class _Tag, class... _As>
        requires __callable<_Tag, __mtype<_Tp>, _Storage&, _As...>
      constexpr void (*operator()(_Tag (*)(void (*)(_As...))) const noexcept)(void*, _As...) {
        return +[](void* __storage, _As... __as) -> void {
          return _Tag{}(__mtype<_Tp>{}, *(_Storage*) __storage, (_As&&) __as...);
        };
      }

      template <class _Tag, class... _As>
        requires __callable<_Tag, __mtype<_Tp>, _Storage&, _As...>
      constexpr void (
        *operator()(_Tag (*)(void (*)(_As...) noexcept)) const noexcept)(void*, _As...) noexcept {
        return +[](void* __storage, _As... __as) noexcept -> void {
          static_assert(__nothrow_callable<_Tag, __mtype<_Tp>, _Storage&, _As...>);
          return _Tag{}(__mtype<_Tp>{}, *(_Storage*) __storage, (_As&&) __as...);
        };
      }
    };

    struct __delete_t {
      template <class _Storage, class _Tp>
        requires tag_invocable<__delete_t, __mtype<_Tp>, _Storage&>
      void operator()(__mtype<_Tp>, _Storage& __storage) noexcept {
        static_assert(nothrow_tag_invocable<__delete_t, __mtype<_Tp>, _Storage&>);
        tag_invoke(__delete_t{}, __mtype<_Tp>{}, __storage);
      }
    };

    inline constexpr __delete_t __delete{};

    struct __copy_construct_t {
      template <class _Storage, class _Tp>
        requires tag_invocable<__copy_construct_t, __mtype<_Tp>, _Storage&, const _Storage&>
      void operator()(__mtype<_Tp>, _Storage& __self, const _Storage& __from) noexcept(
        nothrow_tag_invocable<__copy_construct_t, __mtype<_Tp>, _Storage&, const _Storage&>) {
        tag_invoke(__copy_construct_t{}, __mtype<_Tp>{}, __self, __from);
      }
    };

    inline constexpr __copy_construct_t __copy_construct{};

    struct __move_construct_t {
      template <class _Storage, class _Tp>
        requires tag_invocable<__move_construct_t, __mtype<_Tp>, _Storage&, _Storage&&>
      void operator()(__mtype<_Tp>, _Storage& __self, __midentity<_Storage&&> __from) noexcept {
        static_assert(
          nothrow_tag_invocable<__move_construct_t, __mtype<_Tp>, _Storage&, _Storage&&>);
        tag_invoke(__move_construct_t{}, __mtype<_Tp>{}, __self, (_Storage&&) __from);
      }
    };

    inline constexpr __move_construct_t __move_construct{};

    template <class _ParentVTable, class... _StorageCPOs>
    struct __storage_vtable;

    template <class _ParentVTable, class... _StorageCPOs>
      requires requires { _ParentVTable::operator(); }
    struct __storage_vtable<_ParentVTable, _StorageCPOs...>
      : _ParentVTable
      , __storage_vfun<_StorageCPOs>... {
      using _ParentVTable::operator();
      using __storage_vfun<_StorageCPOs>::operator()...;
    };

    template <class _ParentVTable, class... _StorageCPOs>
      requires(!requires { _ParentVTable::operator(); })
    struct __storage_vtable<_ParentVTable, _StorageCPOs...>
      : _ParentVTable
      , __storage_vfun<_StorageCPOs>... {
      using __storage_vfun<_StorageCPOs>::operator()...;
    };

    template <class _ParentVTable, class... _StorageCPOs>
    inline constexpr __storage_vtable<_ParentVTable, _StorageCPOs...> __null_storage_vtbl{};

    template <class _ParentVTable, class... _StorageCPOs>
    constexpr const __storage_vtable<_ParentVTable, _StorageCPOs...>*
      __default_storage_vtable(__storage_vtable<_ParentVTable, _StorageCPOs...>*) noexcept {
      return &__null_storage_vtbl<_ParentVTable, _StorageCPOs...>;
    }

    template <class _Storage, class _Tp, class _ParentVTable, class... _StorageCPOs>
    static const __storage_vtable<_ParentVTable, _StorageCPOs...> __storage_vtbl{
      {*__create_vtable(__mtype<_ParentVTable>{}, __mtype<_Tp>{})},
      {__storage_vfun_fn<_Storage, _Tp>{}((_StorageCPOs*) nullptr)}...};

    template <
      class _Vtable,
      class _Allocator,
      bool _Copyable = false,
      std::size_t _Alignment = alignof(std::max_align_t),
      std::size_t _InlineSize = 3 * sizeof(void*)>
    struct __storage {
      class __t;
    };

    template <
      class _Vtable,
      class _Allocator,
      bool _Copyable,
      std::size_t _Alignment,
      std::size_t _InlineSize>
    class __storage<_Vtable, _Allocator, _Copyable, _Alignment, _InlineSize>::__t
      : __if_c<_Copyable, __, __move_only> {
      static_assert(
        std::is_convertible_v<typename std::allocator_traits<_Allocator>::void_pointer, void*>);

      static constexpr std::size_t __buffer_size = std::max(_InlineSize, sizeof(void*));
      static constexpr std::size_t __alignment = std::max(_Alignment, alignof(void*));
      using __with_copy = __copy_construct_t(void(const __t&));
      using __with_move = __move_construct_t(void(__t&&) noexcept);
      using __with_delete = __delete_t(void() noexcept);

      template <class _Tp>
      static constexpr bool __is_small = sizeof(_Tp) <= __buffer_size && alignof(_Tp) <= __alignment
                                      && std::is_nothrow_move_constructible_v<_Tp>;

      using __vtable_t = __if_c<
        _Copyable,
        __storage_vtable<_Vtable, __with_delete, __with_move, __with_copy>,
        __storage_vtable<_Vtable, __with_delete, __with_move>>;

      template <class _Tp>
      static constexpr const __vtable_t* __get_vtable_of_type() noexcept {
        if constexpr (_Copyable) {
          return &__storage_vtbl<
            __t,
            __decay_t<_Tp>,
            _Vtable,
            __with_delete,
            __with_move,
            __with_copy>;
        } else {
          return &__storage_vtbl<__t, __decay_t<_Tp>, _Vtable, __with_delete, __with_move>;
        }
      }

     public:
      using __id = __storage;

      __t() = default;

      template <__not_decays_to<__t> _Tp>
        requires __callable<__create_vtable_t, __mtype<_Vtable>, __mtype<__decay_t<_Tp>>>
      __t(_Tp&& __object)
        : __vtable_{__get_vtable_of_type<_Tp>()} {
        using _Dp = __decay_t<_Tp>;
        if constexpr (__is_small<_Dp>) {
          __construct_small<_Dp>((_Tp&&) __object);
        } else {
          __construct_large<_Dp>((_Tp&&) __object);
        }
      }

      template <class _Tp, class... _Args>
        requires __callable<__create_vtable_t, __mtype<_Vtable>, __mtype<_Tp>>
      __t(std::in_place_type_t<_Tp>, _Args&&... __args)
        : __vtable_{__get_vtable_of_type<_Tp>()} {
        if constexpr (__is_small<_Tp>) {
          __construct_small<_Tp>((_Args&&) __args...);
        } else {
          __construct_large<_Tp>((_Args&&) __args...);
        }
      }

      __t(const __t& __other)
        requires(_Copyable)
      {
        (*__other.__vtable_)(__copy_construct, this, __other);
      }

      __t& operator=(const __t& __other)
        requires(_Copyable)
      {
        __t tmp(__other);
        return *this = std::move(tmp);
      }

      __t(__t&& __other) noexcept {
        (*__other.__vtable_)(__move_construct, this, (__t&&) __other);
      }

      __t& operator=(__t&& __other) noexcept {
        __reset();
        (*__other.__vtable_)(__move_construct, this, (__t&&) __other);
        return *this;
      }

      ~__t() {
        __reset();
      }

      void __reset() noexcept {
        (*__vtable_)(__delete, this);
        __object_pointer_ = nullptr;
        __vtable_ = __default_storage_vtable((__vtable_t*) nullptr);
      }

      const _Vtable* __get_vtable() const noexcept {
        return __vtable_;
      }

      void* __get_object_pointer() const noexcept {
        return __object_pointer_;
      }

     private:
      template <class _Tp, class... _As>
      void __construct_small(_As&&... __args) {
        static_assert(sizeof(_Tp) <= __buffer_size && alignof(_Tp) <= __alignment);
        _Tp* __pointer = static_cast<_Tp*>(static_cast<void*>(&__buffer_[0]));
        using _Alloc = typename std::allocator_traits<_Allocator>::template rebind_alloc<_Tp>;
        _Alloc __alloc{__allocator_};
        std::allocator_traits<_Alloc>::construct(__alloc, __pointer, (_As&&) __args...);
        __object_pointer_ = __pointer;
      }

      template <class _Tp, class... _As>
      void __construct_large(_As&&... __args) {
        using _Alloc = typename std::allocator_traits<_Allocator>::template rebind_alloc<_Tp>;
        _Alloc __alloc{__allocator_};
        _Tp* __pointer = std::allocator_traits<_Alloc>::allocate(__alloc, 1);
        try {
          std::allocator_traits<_Alloc>::construct(__alloc, __pointer, (_As&&) __args...);
        } catch (...) {
          std::allocator_traits<_Alloc>::deallocate(__alloc, __pointer, 1);
          throw;
        }
        __object_pointer_ = __pointer;
      }

      template <class _Tp>
      friend void tag_invoke(__delete_t, __mtype<_Tp>, __t& __self) noexcept {
        if (!__self.__object_pointer_) {
          return;
        }
        using _Alloc = typename std::allocator_traits<_Allocator>::template rebind_alloc<_Tp>;
        _Alloc __alloc{__self.__allocator_};
        _Tp* __pointer = static_cast<_Tp*>(std::exchange(__self.__object_pointer_, nullptr));
        std::allocator_traits<_Alloc>::destroy(__alloc, __pointer);
        if constexpr (!__is_small<_Tp>) {
          std::allocator_traits<_Alloc>::deallocate(__alloc, __pointer, 1);
        }
      }

      template <class _Tp>
      friend void
        tag_invoke(__move_construct_t, __mtype<_Tp>, __t& __self, __t&& __other) noexcept {
        if (!__other.__object_pointer_) {
          return;
        }
        _Tp* __pointer = static_cast<_Tp*>(std::exchange(__other.__object_pointer_, nullptr));
        if constexpr (__is_small<_Tp>) {
          _Tp& __other_object = *__pointer;
          __self.template __construct_small<_Tp>((_Tp&&) __other_object);
          using _Alloc = typename std::allocator_traits<_Allocator>::template rebind_alloc<_Tp>;
          _Alloc __alloc{__self.__allocator_};
          std::allocator_traits<_Alloc>::destroy(__alloc, __pointer);
        } else {
          __self.__object_pointer_ = __pointer;
        }
        __self.__vtable_ = std::exchange(
          __other.__vtable_, __default_storage_vtable((__vtable_t*) nullptr));
      }

      template <class _Tp>
        requires _Copyable
      friend void tag_invoke(__copy_construct_t, __mtype<_Tp>, __t& __self, const __t& __other) {
        if (!__other.__object_pointer_) {
          return;
        }
        const _Tp& __other_object = *static_cast<const _Tp*>(__other.__object_pointer_);
        if constexpr (__is_small<_Tp>) {
          __self.template __construct_small<_Tp>(__other_object);
        } else {
          __self.template __construct_large<_Tp>(__other_object);
        }
        __self.__vtable_ = __other.__vtable_;
      }

      const __vtable_t* __vtable_{__default_storage_vtable((__vtable_t*) nullptr)};
      void* __object_pointer_{nullptr};
      alignas(__alignment) std::byte __buffer_[__buffer_size]{};
      STDEXEC_NO_UNIQUE_ADDRESS _Allocator __allocator_{};
    };

    template <class _VTable, class _Allocator = std::allocator<std::byte>>
    using __unique_storage_t = __t<__storage<_VTable, _Allocator>>;

    template <class _VTable, class _Allocator = std::allocator<std::byte>>
    using __copyable_storage_t = __t<__storage<_VTable, _Allocator, true>>;

    namespace __rec {
      template <class _Sig>
      struct __rcvr_vfun;

      template <class _Sigs, class... _Queries>
      struct __vtable {
        class __t;
      };

      template <class _Sigs, class... _Queries>
      struct __ref;

      template <class _Tag, class... _As>
      struct __rcvr_vfun<_Tag(_As...)> {
        void (*__fn_)(void*, _As...) noexcept;
      };

      template <class _Rcvr>
      struct __rcvr_vfun_fn {
        template <class _Tag, class... _As>
        constexpr void (*operator()(_Tag (*)(_As...)) const noexcept)(void*, _As...) noexcept {
          return +[](void* __rcvr, _As... __as) noexcept -> void {
            _Tag{}((_Rcvr&&) *(_Rcvr*) __rcvr, (_As&&) __as...);
          };
        }
      };

      template <class... _Sigs, class... _Queries>
      struct __vtable<completion_signatures<_Sigs...>, _Queries...> {
        class __t
          : public __rcvr_vfun<_Sigs>...
          , public __query_vfun<_Queries>... {
         public:
          using __query_vfun<_Queries>::operator()...;

         private:
          template <class _Rcvr>
            requires receiver_of<_Rcvr, completion_signatures<_Sigs...>>
                  && (__callable<__query_vfun_fn<_Rcvr>, _Queries> && ...)
          friend const __t* tag_invoke(__create_vtable_t, __mtype<__t>, __mtype<_Rcvr>) noexcept {
            static const __t __vtable_{
              {__rcvr_vfun_fn<_Rcvr>{}((_Sigs*) nullptr)}...,
              {__query_vfun_fn<_Rcvr>{}((_Queries) nullptr)}...};
            return &__vtable_;
          }
        };
      };

      template <class... _Sigs, class... _Queries>
      struct __ref<completion_signatures<_Sigs...>, _Queries...> {
       private:
        using __vtable_t = __t<__vtable<completion_signatures<_Sigs...>, _Queries...>>;

        struct __env_t {
          const __vtable_t* __vtable_;
          void* __rcvr_;

          template <class _Tag, class... _As>
            requires __callable<const __vtable_t&, _Tag, void*, _As...>
          friend auto tag_invoke(_Tag, const __env_t& __self, _As&&... __as) noexcept(
            __nothrow_callable<const __vtable_t&, _Tag, void*, _As...>)
            -> __call_result_t<const __vtable_t&, _Tag, void*, _As...> {
            return (*__self.__vtable_)(_Tag{}, __self.__rcvr_, (_As&&) __as...);
          }
        } __env_;
       public:
        using is_receiver = void;

        template <__none_of<__ref, const __ref, __env_t, const __env_t> _Rcvr>
          requires receiver_of<_Rcvr, completion_signatures<_Sigs...>>
                && (__callable<__query_vfun_fn<_Rcvr>, _Queries> && ...)
        __ref(_Rcvr& __rcvr) noexcept
          : __env_{__create_vtable(__mtype<__vtable_t>{}, __mtype<_Rcvr>{}), &__rcvr} {
        }

        template < __completion_tag _Tag, __decays_to<__ref> _Self, class... _As>
          requires __one_of<_Tag(_As...), _Sigs...>
        friend void tag_invoke(_Tag, _Self&& __self, _As&&... __as) noexcept {
          (*static_cast<const __rcvr_vfun<_Tag(_As...)>*>(__self.__env_.__vtable_)->__fn_)(
            ((_Self&&) __self).__env_.__rcvr_, (_As&&) __as...);
        }

        template <std::same_as<__ref> Self>
        friend const __env_t& tag_invoke(get_env_t, const Self& __self) noexcept {
          return __self.__env_;
        }
      };
    } // __rec

    class __operation_vtable {
     public:
      void (*__start_)(void*) noexcept;

     private:
      template <class _Op>
      friend const __operation_vtable*
        tag_invoke(__create_vtable_t, __mtype<__operation_vtable>, __mtype<_Op>) noexcept {
        static __operation_vtable __vtable{[](void* __object_pointer) noexcept -> void {
          STDEXEC_ASSERT(__object_pointer);
          _Op& __op = *static_cast<_Op*>(__object_pointer);
          static_assert(operation_state<_Op>);
          start(__op);
        }};
        return &__vtable;
      }
    };

    using __unique_operation_storage = __unique_storage_t<__operation_vtable>;

    template <class _Sigs, class _Queries>
    using __receiver_ref = __mapply<__mbind_front<__q<__rec::__ref>, _Sigs>, _Queries>;

    template <class _Receiver, class _Sigs, class _Queries>
    struct __operation_base {
      STDEXEC_NO_UNIQUE_ADDRESS _Receiver __receiver_;
    };

    template <class _Sender, class _Receiver, class _Queries>
    struct __operation {
      using _Sigs = completion_signatures_of_t<_Sender>;
      using __receiver_ref_t = __receiver_ref<_Sigs, _Queries>;

      struct __rec {
        using is_receiver = void;
        __operation_base<_Receiver, _Sigs, _Queries>* __op_;

        template < __completion_tag _CPO, __decays_to<__rec> _Self, class... _Args>
          requires __callable<_CPO, _Receiver&&, _Args...>
        friend void tag_invoke(_CPO, _Self&& __self, _Args&&... __args) noexcept {
          _CPO{}((_Receiver&&) __self.__op_->__receiver_, (_Args&&) __args...);
        }

        friend env_of_t<_Receiver> tag_invoke(get_env_t, const __rec& __self) noexcept {
          return get_env(__self.__op_->__receiver_);
        }
      };

      class __t
        : __immovable
        , __operation_base<_Receiver, _Sigs, _Queries> {
       public:
        using __id = __operation;

        __t(_Sender&& __sender, _Receiver&& __receiver)
          : __operation_base<_Receiver, _Sigs, _Queries>{(_Receiver&&) __receiver}
          , __storage_{__sender.__connect(__receiver_ref_t{__rec_})} {
        }

       private:
        __rec __rec_{static_cast<__operation_base<_Receiver, _Sigs, _Queries>*>(this)};
        __unique_operation_storage __storage_{};

        friend void tag_invoke(start_t, __t& __self) noexcept {
          STDEXEC_ASSERT(__self.__storage_.__get_vtable()->__start_);
          __self.__storage_.__get_vtable()->__start_(__self.__storage_.__get_object_pointer());
        }
      };
    };

    template <class _Queries>
    class __query_vtable;

    template <template <class...> class _List, typename... _Queries>
    class __query_vtable<_List<_Queries...>> : public __query_vfun<_Queries>... {
     public:
      using __query_vfun<_Queries>::operator()...;
     private:
      template <class _EnvProvider>
        requires(__callable<__query_vfun_fn<_EnvProvider>, _Queries> && ...)
      friend const __query_vtable*
        tag_invoke(__create_vtable_t, __mtype<__query_vtable>, __mtype<_EnvProvider>) noexcept {
        static const __query_vtable __vtable{
          {__query_vfun_fn<_EnvProvider>{}((_Queries) nullptr)}...};
        return &__vtable;
      }
    };

    template <class _Sigs, class _SenderQueries = __types<>, class _ReceiverQueries = __types<>>
    struct __sender {
      using __receiver_ref_t = __receiver_ref<_Sigs, _ReceiverQueries>;

      class __vtable : public __query_vtable<_SenderQueries> {
       public:
        using __id = __vtable;

        const __query_vtable<_SenderQueries>& __queries() const noexcept {
          return *this;
        }

        __unique_operation_storage (*__connect_)(void*, __receiver_ref_t);
       private:
        template <sender_to<__receiver_ref_t> _Sender>
        friend const __vtable*
          tag_invoke(__create_vtable_t, __mtype<__vtable>, __mtype<_Sender>) noexcept {
          static const __vtable __vtable_{
            {*__create_vtable(__mtype<__query_vtable<_SenderQueries>>{}, __mtype<_Sender>{})},
            [](void* __object_pointer, __receiver_ref_t __receiver) -> __unique_operation_storage {
              _Sender& __sender = *static_cast<_Sender*>(__object_pointer);
              using __op_state_t = connect_result_t<_Sender, __receiver_ref_t>;
              return __unique_operation_storage{
                std::in_place_type<__op_state_t>, __conv{[&] {
                  return connect((_Sender&&) __sender, (__receiver_ref_t&&) __receiver);
                }}};
            }};
          return &__vtable_;
        }
      };

      class __env_t {
       public:
        __env_t(const __vtable* __vtable, void* __sender) noexcept
          : __vtable_{__vtable}
          , __sender_{__sender} {
        }
       private:
        const __vtable* __vtable_;
        void* __sender_;

        template <class _Tag, class... _As>
          requires __callable<const __query_vtable<_SenderQueries>&, _Tag, void*, _As...>
        friend auto tag_invoke(_Tag, const __env_t& __self, _As&&... __as) noexcept(
          __nothrow_callable<const __query_vtable<_SenderQueries>&, _Tag, void*, _As...>)
          -> __call_result_t<const __query_vtable<_SenderQueries>&, _Tag, void*, _As...> {
          return __self.__vtable_->__queries()(_Tag{}, __self.__sender_, (_As&&) __as...);
        }
      };

      class __t {
       public:
        using __id = __sender;
        using completion_signatures = _Sigs;
        using is_sender = void;

        __t(const __t&) = delete;
        __t& operator=(const __t&) = delete;

        __t(__t&&) = default;
        __t& operator=(__t&&) = default;

        template <__not_decays_to<__t> _Sender>
          requires sender_to<_Sender, __receiver_ref<_Sigs, _ReceiverQueries>>
        __t(_Sender&& __sndr)
          : __storage_{(_Sender&&) __sndr} {
        }

        __unique_operation_storage __connect(__receiver_ref_t __receiver) {
          return __storage_.__get_vtable()->__connect_(
            __storage_.__get_object_pointer(), (__receiver_ref_t&&) __receiver);
        }

        explicit operator bool() const noexcept {
          return __get_object_pointer(__storage_) != nullptr;
        }

       private:
        __unique_storage_t<__vtable> __storage_;

        template <receiver_of<_Sigs> _Rcvr>
        friend stdexec::__t<__operation<__t, __decay_t<_Rcvr>, _ReceiverQueries>>
          tag_invoke(connect_t, __t&& __self, _Rcvr&& __rcvr) {
          return {(__t&&) __self, (_Rcvr&&) __rcvr};
        }

        friend __env_t tag_invoke(get_env_t, const __t& __self) noexcept {
          return {__self.__storage_.__get_vtable(), __self.__storage_.__get_object_pointer()};
        }
      };
    };

    template <class _ScheduleSender, class _SchedulerQueries = __types<>>
    class __scheduler {
     public:
      template <class _Scheduler>
        requires(!__decays_to<_Scheduler, __scheduler>) && scheduler<_Scheduler>
      __scheduler(_Scheduler&& __scheduler)
        : __storage_{(_Scheduler&&) __scheduler} {
      }

      using __sender_t = _ScheduleSender;
     private:
      class __vtable : public __query_vtable<_SchedulerQueries> {
       public:
        __sender_t (*__schedule_)(void*) noexcept;
        bool (*__equal_to_)(const void*, const void* other) noexcept;

        const __query_vtable<_SchedulerQueries>& __queries() const noexcept {
          return *this;
        }
       private:
        template <scheduler _Scheduler>
        friend const __vtable*
          tag_invoke(__create_vtable_t, __mtype<__vtable>, __mtype<_Scheduler>) noexcept {
          static const __vtable __vtable_{
            {*__create_vtable(__mtype<__query_vtable<_SchedulerQueries>>{}, __mtype<_Scheduler>{})},
            [](void* __object_pointer) noexcept -> __sender_t {
              const _Scheduler& __scheduler = *static_cast<const _Scheduler*>(__object_pointer);
              return __sender_t{schedule(__scheduler)};
            },
            [](const void* __self, const void* __other) noexcept -> bool {
              static_assert(
                noexcept(std::declval<const _Scheduler&>() == std::declval<const _Scheduler&>()));
              STDEXEC_ASSERT(__self && __other);
              const _Scheduler& __self_scheduler = *static_cast<const _Scheduler*>(__self);
              const _Scheduler& __other_scheduler = *static_cast<const _Scheduler*>(__other);
              return __self_scheduler == __other_scheduler;
            }};
          return &__vtable_;
        }
      };

      template <same_as<__scheduler> _Self>
      friend __sender_t tag_invoke(schedule_t, const _Self& __self) noexcept {
        STDEXEC_ASSERT(__self.__storage_.__get_vtable()->__schedule_);
        return __self.__storage_.__get_vtable()->__schedule_(
          __self.__storage_.__get_object_pointer());
      }

      template <class _Tag, same_as<__scheduler> _Self, class... _As>
        requires __callable<const __query_vtable<_SchedulerQueries>&, _Tag, void*, _As...>
      friend auto tag_invoke(_Tag, const _Self& __self, _As&&... __as) noexcept(
        __nothrow_callable<const __query_vtable<_SchedulerQueries>&, _Tag, void*, _As...>)
        -> __call_result_t<const __query_vtable<_SchedulerQueries>&, _Tag, void*, _As...> {
        return __self.__storage_.__get_vtable()->__queries()(
          _Tag{}, __self.__storage_.__get_object_pointer(), (_As&&) __as...);
      }

      friend bool operator==(const __scheduler& __self, const __scheduler& __other) noexcept {
        if (__self.__storage_.__get_vtable() != __other.__storage_.__get_vtable()) {
          return false;
        }
        void* __p = __self.__storage_.__get_object_pointer();
        void* __o = __other.__storage_.__get_object_pointer();
        // if both object pointers are not null, use the virtual equal_to function
        return (__p && __o && __self.__storage_.__get_vtable()->__equal_to_(__p, __o))
            // if both object pointers are nullptrs, they are equal
            || (!__p && !__o);
      }

      friend bool operator!=(const __scheduler& __self, const __scheduler& __other) noexcept {
        return !(__self == __other);
      }

      __copyable_storage_t<__vtable> __storage_{};
    };
  } // namepsace __any

  template <auto... _Sigs>
  using queries = stdexec::__types<decltype(_Sigs)...>;

  template <class _Completions, auto... _ReceiverQueries>
  class any_receiver_ref {
    using __receiver_base = __any::__rec::__ref<_Completions, decltype(_ReceiverQueries)...>;
    using __env_t = stdexec::env_of_t<__receiver_base>;
    __receiver_base __receiver_;

    template <class _Tag, stdexec::__decays_to<any_receiver_ref> Self, class... _As>
      requires stdexec::tag_invocable< _Tag, stdexec::__copy_cvref_t<Self, __receiver_base>, _As...>
    friend auto tag_invoke(_Tag, Self&& __self, _As&&... __as) noexcept(
      std::is_nothrow_invocable_v< _Tag, stdexec::__copy_cvref_t<Self, __receiver_base>, _As...>) {
      return tag_invoke(_Tag{}, ((Self&&) __self).__receiver_, (_As&&) __as...);
    }

   public:
    using is_receiver = void;
    using __t = any_receiver_ref;
    using __id = any_receiver_ref;

    template <stdexec::__none_of<any_receiver_ref, const any_receiver_ref, __env_t, const __env_t>
                _Receiver>
      requires stdexec::receiver_of<_Receiver, _Completions>
    any_receiver_ref(_Receiver& __receiver) noexcept(
      stdexec::__nothrow_constructible_from<__receiver_base, _Receiver>)
      : __receiver_(__receiver) {
    }

    template <auto... _SenderQueries>
    class any_sender {
      using __sender_base = stdexec::__t<
        __any::__sender<_Completions, queries<_SenderQueries...>, queries<_ReceiverQueries...>>>;
      __sender_base __sender_;

      template <class _Tag, stdexec::__decays_to<any_sender> Self, class... _As>
        requires stdexec::tag_invocable< _Tag, stdexec::__copy_cvref_t<Self, __sender_base>, _As...>
      friend auto tag_invoke(_Tag, Self&& __self, _As&&... __as) noexcept(
        std::is_nothrow_invocable_v< _Tag, stdexec::__copy_cvref_t<Self, __sender_base>, _As...>) {
        return tag_invoke(_Tag{}, ((Self&&) __self).__sender_, (_As&&) __as...);
      }
     public:
      using is_sender = void;
      using completion_signatures = typename __sender_base::completion_signatures;

      template <class _Sender>
        requires(!stdexec::__decays_to<_Sender, any_sender>) && stdexec::sender<_Sender>
      any_sender(_Sender&& __sender) noexcept(
        stdexec::__nothrow_constructible_from<__sender_base, _Sender>)
        : __sender_((_Sender&&) __sender) {
      }

      template <auto... _SchedulerQueries>
      class any_scheduler {
        using __schedule_completions = stdexec::__concat_completion_signatures_t<
          _Completions,
          stdexec::completion_signatures<stdexec::set_value_t()>>;
        using __schedule_receiver = any_receiver_ref<__schedule_completions, _ReceiverQueries...>;

        template <typename _Tag, typename _Sig>
        static _Tag __ret_fn(_Tag (*const)(_Sig));

        template <class _Tag>
        struct __ret_equals_to {
          template <class _Sig>
          using __f = std::is_same<_Tag, decltype(__ret_fn((_Sig) nullptr))>;
        };

        using schedule_sender_queries = stdexec::__minvoke<
          stdexec::__remove_if<
            __ret_equals_to<stdexec::get_completion_scheduler_t<stdexec::set_value_t>>>,
          decltype(_SenderQueries)...>;

        template <class... _Queries>
        using __schedule_sender_fn = typename __schedule_receiver::template any_sender<
          stdexec::get_completion_scheduler<stdexec::set_value_t>.template signature<any_scheduler() noexcept>>;
        using __schedule_sender =
          stdexec::__mapply<stdexec::__q<__schedule_sender_fn>, schedule_sender_queries>;

        using __scheduler_base =
          __any::__scheduler<__schedule_sender, queries<_SchedulerQueries...>>;

        __scheduler_base __scheduler_;
       public:
        using __t = any_scheduler;
        using __id = any_scheduler;

        template <class _Scheduler>
          requires(
            !stdexec::__decays_to<_Scheduler, any_scheduler> && stdexec::scheduler<_Scheduler>)
        any_scheduler(_Scheduler&& __scheduler)
          : __scheduler_{(_Scheduler&&) __scheduler} {
        }

       private:
        template <class _Tag, stdexec::__decays_to<any_scheduler> Self, class... _As>
          requires stdexec::
            tag_invocable< _Tag, stdexec::__copy_cvref_t<Self, __scheduler_base>, _As...>
          friend auto tag_invoke(_Tag, Self&& __self, _As&&... __as) noexcept(
            std::is_nothrow_invocable_v<
              _Tag,
              stdexec::__copy_cvref_t<Self, __scheduler_base>,
              _As...>) {
          return tag_invoke(_Tag{}, ((Self&&) __self).__scheduler_, (_As&&) __as...);
        }

        friend bool
          operator==(const any_scheduler& __self, const any_scheduler& __other) noexcept = default;
      };
    };
  };
} // namespace exec
