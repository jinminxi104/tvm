/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file constant_folding.cc
 */
#include <tvm/relay/analysis.h>
#include <tvm/relay/attrs/transform.h>
#include <tvm/relay/expr_functor.h>
#include <tvm/relay/interpreter.h>
#include <tvm/relay/op.h>
#include <tvm/relay/op_attr_types.h>
#include <tvm/relay/transform.h>
#include <tvm/runtime/container.h>
#include <tvm/runtime/ndarray.h>
#include <tvm/runtime/object.h>


namespace tvm {
namespace relay {

class AddRemover : public ExprMutator{
 public:
  explicit AddRemover(IRModule module)
      : module_(module){}

  Expr VisitExpr_(const CallNode* call_node) final {
      std::cout << "found call node\n";

      if (auto* op = (call_node->op).as<OpNode>()) {
          Expr op_expr = GetRef<Op>(op);

          if (op_expr == Op::Get("add")) {
              std::cout << call_node->op << "\n\n";
              std::cout << call_node->args[0] << "\n\n";
              std::cout << call_node->args[1] << "\n\n";
              return ExprMutator::VisitExpr(call_node->args[0]);
             // return Call(ExprMutator::VisitExpr(call_node->op), {ExprMutator::VisitExpr(call_node->args[0]), Expr(0)});
          }
      }
      return ExprMutator::VisitExpr_(call_node);

  }

 private:
  IRModule module_;
};

Expr RemoveAdd(const Expr& expr, const IRModule& mod) {
  return AddRemover(mod).Mutate(expr);
}

namespace transform {

Pass RemoveAdd() {
  runtime::TypedPackedFunc<Function(Function, IRModule, PassContext)> pass_func =
      [=](Function f, IRModule m, PassContext pc) {
        return Downcast<Function>(RemoveAdd(f, m));
      };
  return CreateFunctionPass(pass_func, 1, "RemoveAdd", {});
}

TVM_REGISTER_GLOBAL("relay._transform.RemoveAdd").set_body_typed(RemoveAdd);

}  // namespace transform

}  // namespace relay
}  // namespace tvm
