// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#pragma once

#include <string>

#include "Python/open3d_pybind.h"

namespace open3d {
namespace docstring {

class ArgumentDoc {
public:
    std::string name_ = "";
    std::string type_ = "";
    std::string default_ = "";
    std::string body_ = "";
};

/// This class is designed to parse docstrings generated by pybind11 and convert
/// to Python friendly Google-style docstring with the flexibility of adding
/// additional docstrings manually.
///
/// The automated part includes:
///     1. Function name
///     2. Argument
///         - Name
///         - Type
///         - Default value
///     3. Return type
///     4. Brief "summary" docstring received from pybind
/// Optionally, the user can inject additional docstrings to the class.
///
/// This approach was chosen in favor of writting docstring in Python files for
/// two reasons 1) We don't need to create additional (pure) Python wrapper
/// functions, 2) The type information generated by pybind is preserved.
///
/// However, this also comes with a drawback: FunctionDoc rely on docstrings
/// generated by pybind11, which is subject to change. So if a new version of
/// pybind11 changes the format of docstring, this class needs to be updated
/// accordingly. Another alternative approach is to modify pybind11 directly,
/// although it makes some of the parsing part simpler, it could be much harder
/// to maintain to keep track of the upstream pybind11.
class FunctionDoc {
public:
    FunctionDoc(const std::string& pybind_doc);

    /// Inject ArgumentDoc body_ docstring
    void inject_argument_doc_body(const std::string& argument_name,
                                  const std::string& argument_doc_body);

    /// Generate Google style python docstring
    std::string to_string() const;

protected:
    /// Parse the function name from docstring
    void parse_function_name();

    /// Parse the function "summary" docstring received from pybind
    void parse_summary();

    /// Parse ArgumentDoc for each argument
    void parse_arguments();

    /// Parse function return
    void parse_return();

protected:
    /// Split docstring to argument tokens
    /// E.g. "cylinder_radius: float = 1.0", "cylinder_radius: float"
    static std::vector<std::string> get_argument_tokens(
            const std::string& pybind_doc);

    /// Parse individual argument token and returns a ArgumentDoc
    static ArgumentDoc parse_argument_token(const std::string& argument_token);

    /// String util: find length of current word staring from a position
    static size_t word_length(const std::string& doc,
                              size_t start_pos,
                              const std::string& valid_chars = "_");

    /// Runs all string cleanup functions
    static std::string str_clean_all(const std::string& s,
                                     const std::string& white_space = " \t\n");

    /// Similar to Python's strip()
    static std::string str_strip(const std::string& s,
                                 const std::string& white_space = " \t\n");

    /// Apply fixes to namespace, e.g. "::" to "." for python
    static std::string namespace_fix(const std::string& s);

public:
    std::string name_ = "";
    std::vector<ArgumentDoc> argument_docs_;
    ArgumentDoc return_doc_;
    std::string summary_ = "";
    std::string body_ = "";

protected:
    std::string pybind_doc_ = "";
};

void FunctionDocInject(py::module& pybind_module,
                       const std::string& function_name,
                       const std::unordered_map<std::string, std::string>
                               map_parameter_docs = {});
}  // namespace docstring
}  // namespace open3d
