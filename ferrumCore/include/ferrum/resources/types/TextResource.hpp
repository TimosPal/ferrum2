#pragma once

#include <string>
#include <utility>

#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources::types {

class TextResource : public ferrum::resources::Resource {
public:
    TextResource(std::string text) : text_(std::move(text)) {}
    TextResource(const TextResource&) = default;
    TextResource& operator=(const TextResource&) = default;
    TextResource(TextResource&&) = default;
    TextResource& operator=(TextResource&&) = default;
    virtual ~TextResource() = default;

    const std::string& GetText() const { return text_; }

private:
    std::string text_;
};

}  // namespace ferrum::resources::types