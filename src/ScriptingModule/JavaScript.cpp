#include "JavaScript.hpp"

IE::Script::detail::JavaScript::JavaScript(IE::Core::Engine *t_engine, IE::Core::File *t_file) :
        Script(t_engine, t_file) {
}

std::shared_ptr<IE::Script::Script>
IE::Script::detail::JavaScript::create(IE::Core::Engine *t_engine, IE::Core::File *t_resource) {
    return std::make_shared<IE::Script::detail::JavaScript>(t_engine, t_resource);
}

void IE::Script::detail::JavaScript::update() {
}

void IE::Script::detail::JavaScript::initialize() {
}

void IE::Script::detail::JavaScript::load() {
}

void IE::Script::detail::JavaScript::compile() {
}
