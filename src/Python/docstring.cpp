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

#include <string>
#include <unordered_map>
#include <tuple>
#include <unordered_set>
#include <sstream>
#include <regex>

#include "Python/docstring.h"

namespace open3d {
namespace docstring {

// Search and replace in string
static std::string str_replace(std::string s,
                               const std::string& search,
                               const std::string& replace) {
    // https://stackoverflow.com/a/14679003/1255535
    size_t pos = 0;
    while ((pos = s.find(search, pos)) != std::string::npos) {
        s.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return s;
}

// Deduplicate namespace (optional)
static std::string namespace_dedup(const std::string& s) {
    return str_replace(s, "open3d.open3d", "open3d");
}

// Similar to Python's str.strip()
static std::string str_strip(const std::string& s,
                             const std::string& white_space = " \t\n") {
    size_t begin_pos = s.find_first_not_of(white_space);
    if (begin_pos == std::string::npos) {
        return "";
    }
    size_t end_pos = s.find_last_not_of(white_space);
    return s.substr(begin_pos, end_pos - begin_pos + 1);
}

// Count the length of current word starting from start_pos
static size_t word_length(const std::string& doc,
                          size_t start_pos,
                          const std::string& valid_chars = "_") {
    std::unordered_set<char> valid_chars_set;
    for (const char& c : valid_chars) {
        valid_chars_set.insert(c);
    }
    auto is_word_char = [&valid_chars_set](const char& c) {
        return std::isalnum(c) ||
               valid_chars_set.find(c) != valid_chars_set.end();
    };
    size_t length = 0;
    for (size_t pos = start_pos; pos < doc.size(); ++pos) {
        if (!is_word_char(doc[pos])) {
            break;
        }
        length++;
    }
    return length;
}

// Splits
// "create_mesh_arrow(cylinder_radius: float = 1.0) -> geometry.TriangleMesh"
// to
// ("create_mesh_arrow(cylinder_radius: float = 1.0)",
//  "geometry.TriangleMesh")
static std::pair<std::string, std::string> split_arrow(const std::string& doc) {
    std::size_t arrow_pos = doc.rfind(" -> ");
    if (arrow_pos != std::string::npos) {
        std::string func_name_and_params = doc.substr(0, arrow_pos);
        std::string return_type = doc.substr(
                arrow_pos + 4, word_length(doc, arrow_pos + 4, "._:"));
        return std::make_pair(namespace_dedup(str_strip(func_name_and_params)),
                              namespace_dedup(str_strip(return_type)));
    } else {
        return std::make_pair(doc, "");
    }
}

static void parse_function_name(const std::string& pybind_doc,
                                FunctionDoc& function_doc) {
    size_t parenthesis_pos = pybind_doc.find("(");
    if (parenthesis_pos == std::string::npos) {
        return;
    } else {
        std::string name = pybind_doc.substr(0, parenthesis_pos);
        function_doc.name_ = name;
    }
}

// Parse docstring for a single argument
// E.g. "cylinder_radius: float = 1.0"
// E.g. "cylinder_radius: float"
static ArgumentDoc parse_single_argument(const std::string& argument_str) {
    ArgumentDoc argument_doc;
    return argument_doc;
}

void parse_regex_dummy() {
    std::regex pattern("(, [A-Za-z_][A-Za-z\\d_]*:)");
    std::smatch res;
    std::string str =
            "hello, cylinder_radius: float = 1.0, cone_s34plit: int = 1, "
            "kim_il_sung5: int = 2";

    std::string::const_iterator start_iter(str.cbegin());
    while (std::regex_search(start_iter, str.cend(), res, pattern)) {
        std::cout << res[0] << std::endl;
        size_t pos = res.position(0) + (start_iter - str.cbegin());
        start_iter = res.suffix().first;
        std::cout << "location " << pos << std::endl;
    }
}

// Parse docstrings of arguments
// Input: "foo(arg0: float, arg1: float = 1.0, arg2: int = 1) -> open3d.bar"
// Goal: split to {"arg0: float", "arg1: float = 1.0", "arg2: int = 1"} and call
//       parse_single_argument respectively
static void parse_doc_arguments(const std::string& pybind_doc,
                                FunctionDoc& function_doc) {
    // First insert commas to make things easy
    // "foo(, arg0: float, arg1: float = 1.0, arg2: int = 1, ) -> open3d.bar"
    std::string doc = pybind_doc;
    size_t parenthesis_pos = doc.find("(");
    if (parenthesis_pos == std::string::npos) {
        return;
    } else {
        doc.replace(parenthesis_pos + 1, 0, ", ");
    }
    std::size_t arrow_pos = doc.rfind(") -> ");
    if (arrow_pos == std::string::npos) {
        return;
    } else {
        doc.replace(arrow_pos, 0, ", ");
    }

    // Find begin index of each argument
}

static void parse_doc_result(const std::string& pybind_doc,
                             FunctionDoc& function_doc) {
    std::size_t arrow_pos = pybind_doc.rfind(" -> ");
    if (arrow_pos != std::string::npos) {
        std::string return_type = pybind_doc.substr(
                arrow_pos + 4, word_length(pybind_doc, arrow_pos + 4, "._:"));
        function_doc.return_doc_.type_ = return_type;
    }
}

// Currently copied this function for testing
// TODO: link unit test with python module to enable direct testing
static FunctionDoc parse_doc_function(const std::string& pybind_doc) {
    FunctionDoc function_doc;

    // Split by "->"
    std::string func_name_and_arguments;
    std::string return_type;
    std::tie(func_name_and_arguments, return_type) = split_arrow(pybind_doc);

    parse_function_name(pybind_doc, function_doc);
    parse_doc_arguments(pybind_doc, function_doc);
    parse_doc_result(pybind_doc, function_doc);

    return function_doc;
}

void function_doc_inject(
        py::module& pybind_module,
        const std::string& function_name,
        const std::unordered_map<std::string, std::string> map_parameter_docs) {
    // Get function
    PyObject* module = pybind_module.ptr();
    PyObject* f_obj = PyObject_GetAttrString(module, function_name.c_str());
    if (Py_TYPE(f_obj) != &PyCFunction_Type) {
        return;
    }
    PyCFunctionObject* f = (PyCFunctionObject*)f_obj;
    // f->m_ml->ml_doc = "read_feature(filenamelalala)";

    std::string doc = R"(
create_mesh_arrow(cylinder_radius: float = 1.0, cone_split: int = 1) -> open3d.open3d.geometry.TriangleMesh

Factory function to create an arrow mesh
)";
    std::cout << parse_doc_function(doc).to_string();

    // std::cout << "f->m_ml->ml_doc " << f->m_ml->ml_doc << std::endl;
    std::cout << parse_doc_function(f->m_ml->ml_doc).to_string();
}

std::string FunctionDoc::to_string() const {
    // Example Gooele style:
    // http://www.sphinx-doc.org/en/1.5/ext/example_google.html

    std::ostringstream rc;
    std::string indent = "    ";

    // Function signature to be parsed by Sphinx
    rc << name_ << "(";
    for (size_t i = 0; i < argument_docs_.size(); ++i) {
        const ArgumentDoc& argument_doc = argument_docs_[i];
        rc << argument_doc.name_;
        if (argument_doc.default_ != "") {
            rc << "=" << argument_doc.default_;
        }
        if (i != argument_docs_.size() - 1) {
            rc << ", ";
        }
    }
    rc << ")" << std::endl;

    // Summary line, strictly speaking this shall be at the very front. However
    // from a compiled Python module we need the function signature hints in
    // front for Sphinx parsing and PyCharm autocomplete
    if (summary_ != "") {
        rc << std::endl;
        rc << summary_ << std::endl;
    }

    // Arguments
    if (argument_docs_.size() != 0) {
        rc << std::endl;
        rc << "Args:" << std::endl;
        for (const ArgumentDoc& argument_doc : argument_docs_) {
            rc << indent << argument_doc.name_ << "(" << argument_doc.type_
               << "): " << argument_doc.body_ << std::endl;
        }
    }

    // Return
    rc << std::endl;
    rc << "Returns:" << std::endl;
    rc << indent << return_doc_.type_;
    if (return_doc_.body_ != "") {
        rc << ": " << return_doc_.body_;
    }
    rc << std::endl;

    return rc.str();
}

}  // namespace docstring
}  // namespace open3d
