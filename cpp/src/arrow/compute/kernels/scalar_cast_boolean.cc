// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// Cast types to boolean

#include "arrow/compute/kernels/common.h"
#include "arrow/compute/kernels/scalar_cast_internal.h"
#include "arrow/util/value_parsing.h"

namespace arrow {

using internal::ParseValue;

namespace compute {
namespace internal {

struct IsNonZero {
  template <typename OUT, typename ARG0>
  static OUT Call(KernelContext*, ARG0 val) {
    return val != 0;
  }
};

struct ParseBooleanString {
  template <typename OUT, typename ARG0>
  static OUT Call(KernelContext* ctx, ARG0 val) {
    bool result = false;
    if (ARROW_PREDICT_FALSE(!ParseValue<BooleanType>(val.data(), val.size(), &result))) {
      ctx->SetStatus(Status::Invalid("Failed to parse value: ", val));
    }
    return result;
  }
};

std::vector<std::shared_ptr<CastFunction>> GetBooleanCasts() {
  auto func = std::make_shared<CastFunction>("cast_boolean", Type::BOOL);
  AddCommonCasts<BooleanType>(boolean(), func.get());

  for (const auto& ty : NumericTypes()) {
    ArrayKernelExec exec =
        codegen::Numeric<codegen::ScalarUnary, BooleanType, IsNonZero>(*ty);
    DCHECK_OK(func->AddKernel(ty->id(), {ty}, boolean(), exec));
  }
  for (const auto& ty : BaseBinaryTypes()) {
    ArrayKernelExec exec =
        codegen::BaseBinary<codegen::ScalarUnaryNotNull, BooleanType, ParseBooleanString>(
            *ty);
    DCHECK_OK(func->AddKernel(ty->id(), {ty}, boolean(), exec));
  }
  return {func};
}

}  // namespace internal
}  // namespace compute
}  // namespace arrow
