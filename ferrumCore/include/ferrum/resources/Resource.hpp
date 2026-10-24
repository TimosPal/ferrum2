#pragma once

namespace ferrum::resources {

class Resource {
public:
    Resource() = default;
    Resource(const Resource&) = default;
    Resource& operator=(const Resource&) = default;
    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;
    virtual ~Resource() = default;
};

}  // namespace ferrum::resources