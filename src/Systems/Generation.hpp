#pragma once

#include "../System.hpp"

#include <memory>
#include <WebSockets/WebSocket.hpp>

class Generation : public System {
    // Lifecycle Methods
public:
    ~Generation() noexcept;
    Generation(const Generation&) = delete;
    Generation(Generation&&) noexcept = delete;
    Generation& operator=(const Generation&) = delete;
    Generation& operator=(Generation&&) noexcept = delete;

    // Public Methods
public:
    /**
     * This is the constructor of the class.
     */
    Generation();

    // System
public:
    virtual void Update(
        Components& components,
        size_t tick
    ) override;

    // Private properties
private:
    /**
     * This is the type of structure that contGenerationns the private
     * properties of the instance.  It is defined in the implementation
     * and declared here to ensure that it is scoped inside the class.
     */
    struct Impl;

    /**
     * This contGenerationns the private properties of the instance.
     */
    std::unique_ptr< Impl > impl_;
};
