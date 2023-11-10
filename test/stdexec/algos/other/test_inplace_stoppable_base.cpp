/*
 * Copyright (c) 2024 Lauri Vasama
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

#include <catch2/catch.hpp>
#include <stdexec/execution.hpp>

namespace ex = stdexec;

namespace {

  template <class StopToken>
  class stoppable : ex::__stoppable_base_for_t<StopToken, stoppable<StopToken>>
  {
    using base = ex::__stoppable_base_for_t<StopToken, stoppable<StopToken>>;

   public:
    explicit stoppable(StopToken token, bool& stop_requested)
      : base(std::move(token))
      , stop_requested_(stop_requested)
    {
      base::register_stoppable();
    }

   private:
    bool& stop_requested_;

    void on_stop_requested()
    {
      stop_requested_ = true;
    }

    friend ex::__stoppable_base_for_t<StopToken, stoppable<StopToken>>;
  };

  using inplace_stoppable = stoppable<ex::inplace_stop_token>;

  TEST_CASE(
    "request_stop after registration signals inplace_stoppable_base",
    "[other][inplace_stoppable_base]") {
    ex::inplace_stop_source source;

    bool stop_requested = false;
    inplace_stoppable s(source.get_token(), stop_requested);
    REQUIRE(!stop_requested);

    source.request_stop();
    REQUIRE(stop_requested);
  }

  TEST_CASE(
    "request_stop before registration signals inplace_stoppable_base",
    "[other][inplace_stoppable_base]") {
    ex::inplace_stop_source source;
    source.request_stop();

    bool stop_requested = false;
    inplace_stoppable s(source.get_token(), stop_requested);
    REQUIRE(stop_requested);
  }

  TEST_CASE(
    "request_stop after deregistration does not signal inplace_stoppable_base",
    "[other][inplace_stoppable_base]") {
    ex::inplace_stop_source source;

    bool stop_requested = false;
    { inplace_stoppable s(source.get_token(), stop_requested); }

    source.request_stop();
    REQUIRE(!stop_requested);
  }

  #if 0
  TEST_CASE(
    "stoppable_base_for works with std::stop_token",
    "[other][stoppable_base_for]") {
    std::stop_source source;

    bool stop_requested = false;
    stoppable<std::stop_token> s(source.get_token(), stop_requested);
    REQUIRE(!stop_requested);

    source.request_stop();
    REQUIRE(stop_requested);
  }
  #endif

} // namespace
