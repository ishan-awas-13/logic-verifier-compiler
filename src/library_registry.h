#pragma once

#include "types.h"
#include <string>
#include <vector>

namespace cmm {

enum class LibUseKind {
    Type,
    Function
};

struct LibUse {
    std::string symbol;
    LibUseKind kind;
    std::string provider; // provider library name or empty
    int line;
    int col;
};

struct FunctionDef {
    std::string name;
    std::vector<Type> param_types;
    Type return_type;
    std::string intrinsic_id;
};

struct LibraryDef {
    std::string name;
    std::string description;
    std::vector<Type> provided_types;
    std::vector<FunctionDef> functions;
};

class LibraryRegistry {
public:
    LibraryRegistry();
    explicit LibraryRegistry(std::vector<LibraryDef> libraries);

    static const LibraryRegistry& default_registry();

    const LibraryDef* findLibrary(const std::string& name) const;
    std::string providerOfType(Type type) const;
    const FunctionDef* findFunction(const std::string& name) const;
    std::string providerOfFunction(const std::string& name) const;
    std::vector<std::string> allLibraryNames() const;
    std::vector<std::string> allFunctionNames() const;
    const std::vector<LibraryDef>& get_libraries() const { return libraries_; }

    void addLibrary(LibraryDef lib);

private:
    std::vector<LibraryDef> libraries_;
};

} // namespace cmm
