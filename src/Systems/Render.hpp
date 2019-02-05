#pragma once

#include "../System.hpp"

#include <memory>
#include <WebSockets/WebSocket.hpp>

class Render : public System {
    // Lifecycle Methods
public:
    ~Render() noexcept;
    Render(const Render&) = delete;
    Render(Render&&) noexcept = delete;
    Render& operator=(const Render&) = delete;
    Render& operator=(Render&&) noexcept = delete;

    // Public Methods
public:
    /**
     * This is the constructor of the class.
     */
    Render();

    void SetClient(std::shared_ptr< WebSockets::WebSocket > ws);

    // System
public:
    virtual void Update(Components& components) override;

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
