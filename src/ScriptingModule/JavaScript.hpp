#pragma once

#include <ScriptingModule/Script.hpp>

namespace IE::Script::detail {
class JavaScript : public IE::Script::Script {
public:
    JavaScript(IE::Core::Engine *t_engine, IE::Core::File *t_file);
    void                                       update() override;
    void                                       initialize() override;
    void                                       load() override;
    void                                       compile() override;
    static std::shared_ptr<IE::Script::Script> create(IE::Core::Engine *t_engine, IE::Core::File *t_file);
};
}  // namespace IE::Script::detail