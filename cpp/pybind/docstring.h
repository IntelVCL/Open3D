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
#include <unordered_map>

#include "pybind/open3d_pybind.h"

namespace open3d {
namespace docstring {

class ArgumentDoc {
public:
    std::string name_ = "";
    std::string type_ = "";
    std::string default_ = "";
    // Long default values are not displayed in signature, but in docstrings
    std::string long_default_ = "";
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
///
/// Example usage:
///
/// ```cpp
/// std::string doc = R"(foo(arg0: int, arg1: float = 1.0) -> open3d.bar)";
/// FunctionDoc fd(doc);
/// fd.InjectArgumentDocBody("arg0", "The arg0 is an important argument.");
/// std::cout << fd.ToGoogleDocString();
/// ```
///
class FunctionDoc {
public:
    FunctionDoc(const std::string& pybind_doc);

    /// Generate Google style python docstring.
    std::string ToGoogleDocString() const;

    /// Apply fixes to namespace, e.g. "::" to "." for python
    static std::string NamespaceFix(const std::string& s);

protected:
    /// Parse the function name from docstring.
    ///
    /// \returns Position in docstring after the name or after the string
    /// "Overloaded function.", if it exists.
    size_t ParseFunctionName();

    /// Parse the function "summary" docstring received from pybind.
    ///
    /// \returns Position in docstring at the end of the summary. Used to set
    /// limits for parsing the current overload.
    size_t ParseSummary();

    /// Parse ArgumentDoc for each argument.
    void ParseArguments();

    /// Parse function return.
    void ParseReturn();

    /// Split docstring to argument tokens.
    /// E.g. "cylinder_radius: float = 1.0", "cylinder_radius: float"
    static std::vector<std::string> GetArgumentTokens(
            const std::string& pybind_doc);

    /// Parse individual argument token and returns a ArgumentDoc.
    static ArgumentDoc ParseArgumentToken(const std::string& argument_token);

    /// Runs all string cleanup functions.
    static std::string StringCleanAll(std::string& s,
                                      const std::string& white_space = " \t\n");

public:
    struct OverloadDocs {
        std::vector<ArgumentDoc> argument_docs_;
        ArgumentDoc return_doc_;
        std::string summary_ = "";
    };
    std::string name_ = "";
    std::string preamble_ = "";
    std::vector<OverloadDocs> overload_docs_;

protected:
    std::string pybind_doc_ = "";
    size_t doc_pos_[2] = {0, std::string::npos};
};

/// Parse pybind docstring to FunctionDoc and inject argument docstrings for
/// functions.
void FunctionDocInject(
        py::module& pybind_module,
        const std::string& function_name,
        const std::unordered_map<std::string, std::string>& map_parameter_docs =
                std::unordered_map<std::string, std::string>());

/// Parse pybind docstring to FunctionDoc and inject argument docstrings for
/// class methods.
void ClassMethodDocInject(
        py::module& pybind_module,
        const std::string& class_name,
        const std::string& function_name,
        const std::unordered_map<std::string, std::string>&
                map_parameter_body_docs =
                        std::unordered_map<std::string, std::string>());

extern py::handle static_property;

}  // namespace docstring
}  // namespace open3d
