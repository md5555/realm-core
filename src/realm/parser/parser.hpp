////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#ifndef REALM_PARSER_HPP
#define REALM_PARSER_HPP

#include <vector>
#include <string>

namespace realm {

namespace parser {
struct Expression
{
    enum class Type { None, Number, String, KeyPath, Argument, True, False, Null, Timestamp } type;
    enum class KeyPathOp { None, Min, Max, Avg, Sum, Count, Size } collection_op;
    std::string s;
    std::vector<std::string> time_inputs;
    std::string op_suffix;
    Expression(Type t = Type::None, std::string input = "") : type(t), collection_op(KeyPathOp::None), s(input) {}
    Expression(std::vector<std::string>&& timestamp) : type(Type::Timestamp), collection_op(KeyPathOp::None), time_inputs(timestamp) {}
    Expression(std::string prefix, KeyPathOp op, std::string suffix) : type(Type::KeyPath), collection_op(op), s(prefix), op_suffix(suffix) {}
};

struct Predicate
{
    enum class Type
    {
        Comparison,
        Or,
        And,
        True,
        False
    } type = Type::And;

    enum class Operator
    {
        None,
        Equal,
        NotEqual,
        LessThan,
        LessThanOrEqual,
        GreaterThan,
        GreaterThanOrEqual,
        BeginsWith,
        EndsWith,
        Contains,
        Like
    };

    enum class OperatorOption
    {
        None,
        CaseInsensitive,
    };

    struct Comparison
    {
        Operator op = Operator::None;
        OperatorOption option = OperatorOption::None;
        Expression expr[2] = {{Expression::Type::None, ""}, {Expression::Type::None, ""}};
    };

    struct Compound
    {
        std::vector<Predicate> sub_predicates;
    };

    Comparison cmpr;
    Compound   cpnd;

    bool negate = false;

    Predicate(Type t, bool n = false) : type(t), negate(n) {}
};

Predicate parse(const std::string &query);

void analyze_grammar();
bool test_grammar();
}
}

#endif // REALM_PARSER_HPP