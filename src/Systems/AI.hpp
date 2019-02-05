#pragma once

#include "../System.hpp"

#include <memory>
#include <WebSockets/WebSocket.hpp>

class AI : public System {
    // Lifecycle Methods
public:
    ~AI() noexcept;
    AI(const AI&) = delete;
    AI(AI&&) noexcept = delete;
    AI& operator=(const AI&) = delete;
    AI& operator=(AI&&) noexcept = delete;

    // Public Methods
public:
    /**
     * This is the constructor of the class.
     */
    AI();

    // System
public:
    virtual void Update(
        Components& components,
        size_t tick
    ) override;

    // Private properties
private:
    /**
     * This is the type of structure that contains the private
     * properties of the instance.  It is defined in the implementation
     * and declared here to ensure that it is scoped inside the class.
     */
    struct Impl;

    /**
     * This contains the private properties of the instance.
     */
    std::unique_ptr< Impl > impl_;
};
