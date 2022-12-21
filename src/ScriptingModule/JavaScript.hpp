#pragma once

#include <ScriptingModule/Script.hpp>

namespace IE::Script::detail {
class JavaScript : public IE::Script::Script {
public:
    JavaScript(std::shared_ptr<IE::Core::Engine> t_engine, IE::Core::File *t_file);
    void                                update() override;
    void                                initialize() override;
    void                                load() override;
    void                                compile() override;
    std::shared_ptr<IE::Script::Script> create(std::shared_ptr<IE::Core::Engine> t_engine, IE::Core::File *t_file);
};
}  // namespace IE::Script::detail