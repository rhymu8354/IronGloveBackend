#pragma once

/**
 * @file game.hpp
 *
 * This module declares the Game implementation.
 *
 * © 2019 by Richard Walters
 */

#include <functional>
#include <memory>
#include <SystemAbstractions/DiagnosticsSender.hpp>
#include <WebSockets/WebSocket.hpp>

class Game {
    // Types
public:
    using CompleteDelegate = std::function<
        void()
    >;

    // Lifecycle Methods
public:
    ~Game() noexcept;
    Game(const Game&) = delete;
    Game(Game&&) noexcept = delete;
    Game& operator=(const Game&) = delete;
    Game& operator=(Game&&) noexcept = delete;

    // Public Methods
public:
    /**
     * This is the constructor of the class.
     */
    explicit Game(const std::string& id);

    void Start(
        std::shared_ptr< WebSockets::WebSocket > ws,
        SystemAbstractions::DiagnosticsSender::DiagnosticMessageDelegate diagnosticMessageDelegate,
        CompleteDelegate completeDelegate
    );

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
    std::shared_ptr< Impl > impl_;
};
