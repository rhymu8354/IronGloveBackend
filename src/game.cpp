#include "Components.hpp"
#include "game.hpp"
#include "Systems.hpp"

#include <future>
#include <Json/Value.hpp>
#include <string>
#include <SystemAbstractions/DiagnosticsSender.hpp>
#include <thread>

struct Game::Impl
    : public std::enable_shared_from_this< Game::Impl >
{
    std::shared_ptr< WebSockets::WebSocket > ws;
    CompleteDelegate completeDelegate;
    SystemAbstractions::DiagnosticsSender diagnosticsSender;
    Components components;
    SystemCollection systems;
    std::promise< void > stopWorker;
    std::thread worker;

    explicit Impl(const std::string& id)
        : diagnosticsSender(id)
    {
    }

    void OnWebSocketClosed() {
        diagnosticsSender.SendDiagnosticInformationString(
            3,
            "Goodbye!"
        );
        stopWorker.set_value();
        worker.join();
        completeDelegate();
    }

    void OnWebSocketText(const std::string& data) {
        const auto message = Json::Value::FromEncoding(data);
        if (message["type"] == "keyDown") {
            const auto keyString = (std::string)message["key"];
            if (keyString.empty()) {
                return;
            }
            const auto key = keyString[0];
            diagnosticsSender.SendDiagnosticInformationString(
                3,
                std::string("Key Down: ") + key
            );
        } else if (message["type"] == "keyUp") {
            const auto keyString = (std::string)message["key"];
            if (keyString.empty()) {
                return;
            }
            const auto key = keyString[0];
            diagnosticsSender.SendDiagnosticInformationString(
                3,
                std::string("Key Up: ") + key
            );
        } else if (message["type"] == "hello") {
        }
    }

    void SetWebSocketDelegates() {
        WebSockets::WebSocket::Delegates delegates;
        std::weak_ptr< Impl > implWeak(shared_from_this());
        delegates.close = [implWeak](
            unsigned int code,
            const std::string& reason
        ){
            const auto impl = implWeak.lock();
            if (impl == nullptr) {
                return;
            }
            impl->OnWebSocketClosed();
        };
        delegates.text = [implWeak](
            const std::string& data
        ){
            const auto impl = implWeak.lock();
            if (impl == nullptr) {
                return;
            }
            impl->OnWebSocketText(data);
        };
        ws->SetDelegates(std::move(delegates));
    }

    void AddPlayer(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "hero";
        position->x = x;
        position->y = y;
    }

    void AddMonster(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "monster";
        position->x = x;
        position->y = y;
    }

    void Worker() {
        diagnosticsSender.SendDiagnosticInformationString(
            3,
            "Worker started!"
        );
        auto workerToldToStop = stopWorker.get_future();
        while (
            workerToldToStop.wait_for(std::chrono::milliseconds(2000))
            != std::future_status::ready
        ) {
            for (const auto system: systems) {
                system->Update(components);
            }
        }
        diagnosticsSender.SendDiagnosticInformationString(
            3,
            "Worker stopped!"
        );
    }
};

Game::~Game() = default;
Game::Game(const std::string& id)
    : impl_(new Impl(id))
{
}

void Game::Start(
    std::shared_ptr< WebSockets::WebSocket > ws,
    SystemAbstractions::DiagnosticsSender::DiagnosticMessageDelegate diagnosticMessageDelegate,
    CompleteDelegate completeDelegate
) {
    impl_->diagnosticsSender.SubscribeToDiagnostics(diagnosticMessageDelegate);
    impl_->diagnosticsSender.SendDiagnosticInformationString(
        3,
        "Try this level now!"
    );
    impl_->ws = ws;
    impl_->completeDelegate = completeDelegate;
    impl_->systems = Systems(ws);
    impl_->AddPlayer(0, 0);
    impl_->AddMonster(1, 0);
    impl_->AddMonster(0, 1);
    impl_->SetWebSocketDelegates();
    impl_->worker = std::thread(&Impl::Worker, impl_.get());
}
