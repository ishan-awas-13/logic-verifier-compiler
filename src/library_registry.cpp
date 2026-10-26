#include "library_registry.h"
#include <algorithm>

namespace cmm {

static std::vector<LibraryDef> create_default_libraries() {
    std::vector<LibraryDef> libs;

    // text library
    LibraryDef text_lib;
    text_lib.name = "text";
    text_lib.description = "String and character manipulation";
    text_lib.provided_types = { Type::Word };
    text_lib.functions = {
        FunctionDef{"length", { Type::Word }, Type::Num, "text.length"},
        FunctionDef{"isempty", { Type::Word }, Type::Boolean, "text.isempty"}
    };
    libs.push_back(std::move(text_lib));

    // vector library (gates type list only, size(list) omitted as lists not supported in runtime)
    LibraryDef vec_lib;
    vec_lib.name = "vector";
    vec_lib.description = "Dynamic list and collection handling";
    vec_lib.provided_types = { Type::List };
    libs.push_back(std::move(vec_lib));

    // math library
    LibraryDef math_lib;
    math_lib.name = "math";
    math_lib.description = "Standard mathematical functions";
    math_lib.functions = {
        FunctionDef{"absolute", { Type::Num }, Type::Num, "math.absolute"},
        FunctionDef{"maximum", { Type::Num, Type::Num }, Type::Num, "math.maximum"},
        FunctionDef{"minimum", { Type::Num, Type::Num }, Type::Num, "math.minimum"}
    };
    libs.push_back(std::move(math_lib));

    return libs;
}

LibraryRegistry::LibraryRegistry() : libraries_(create_default_libraries()) {}

LibraryRegistry::LibraryRegistry(std::vector<LibraryDef> libraries) : libraries_(std::move(libraries)) {}

const LibraryRegistry& LibraryRegistry::default_registry() {
    static const LibraryRegistry instance;
    return instance;
}

const LibraryDef* LibraryRegistry::findLibrary(const std::string& name) const {
    for (const auto& lib : libraries_) {
        if (lib.name == name) {
            return &lib;
        }
    }
    return nullptr;
}

std::string LibraryRegistry::providerOfType(Type type) const {
    for (const auto& lib : libraries_) {
        for (Type t : lib.provided_types) {
            if (t == type) {
                return lib.name;
            }
        }
    }
    return "";
}

const FunctionDef* LibraryRegistry::findFunction(const std::string& name) const {
    for (const auto& lib : libraries_) {
        for (const auto& fn : lib.functions) {
            if (fn.name == name) {
                return &fn;
            }
        }
    }
    return nullptr;
}

std::string LibraryRegistry::providerOfFunction(const std::string& name) const {
    for (const auto& lib : libraries_) {
        for (const auto& fn : lib.functions) {
            if (fn.name == name) {
                return lib.name;
            }
        }
    }
    return "";
}

std::vector<std::string> LibraryRegistry::allLibraryNames() const {
    std::vector<std::string> names;
    names.reserve(libraries_.size());
    for (const auto& lib : libraries_) {
        names.push_back(lib.name);
    }
    return names;
}

std::vector<std::string> LibraryRegistry::allFunctionNames() const {
    std::vector<std::string> names;
    for (const auto& lib : libraries_) {
        for (const auto& fn : lib.functions) {
            names.push_back(fn.name);
        }
    }
    return names;
}

void LibraryRegistry::addLibrary(LibraryDef lib) {
    libraries_.push_back(std::move(lib));
}

} // namespace cmm

