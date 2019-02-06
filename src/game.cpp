#include "Components.hpp"
#include "game.hpp"
#include "Systems.hpp"

#include <future>
#include <Json/Value.hpp>
#include <mutex>
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
    std::mutex mutex;
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
        std::lock_guard< decltype(mutex) > lock(mutex);
        const auto inputsInfo = components.GetComponentsOfType(Components::Type::Input);
        if (inputsInfo.n == 0) {
            return;
        }
        auto& input = ((Input*)inputsInfo.first)[0];
        const auto message = Json::Value::FromEncoding(data);
        if (message["type"] == "fire") {
            const auto keyString = (std::string)message["key"];
            if (keyString.empty()) {
                input.fire = 0;
            } else {
                const auto key = keyString[0];
                input.fire = key;
            }
        } else if (message["type"] == "move") {
            const auto keyString = (std::string)message["key"];
            if (keyString.empty()) {
                input.move = 0;
            } else {
                const auto key = keyString[0];
                input.move = key;
            }
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
        const auto collider = (Collider*)components.CreateComponentOfType(Components::Type::Collider, id);
        const auto input = (Input*)components.CreateComponentOfType(Components::Type::Input, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "hero";
        tile->z = 1;
        position->x = x;
        position->y = y;
    }

    void AddMonster(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto collider = (Collider*)components.CreateComponentOfType(Components::Type::Collider, id);
        const auto monster = (Monster*)components.CreateComponentOfType(Components::Type::Monster, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "monster";
        tile->z = 1;
        position->x = x;
        position->y = y;

    void AddWall(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto collider = (Collider*)components.CreateComponentOfType(Components::Type::Collider, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "wall";
        tile->z = 1;
        position->x = x;
        position->y = y;
    }

    void AddFloor(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "floor";
        tile->z = 0;
        position->x = x;
        position->y = y;
    }

    void Worker() {
        diagnosticsSender.SendDiagnosticInformationString(
            3,
            "Worker started!"
        );
        auto workerToldToStop = stopWorker.get_future();
        size_t tick = 0;
        while (
            workerToldToStop.wait_for(std::chrono::milliseconds(250))
            != std::future_status::ready
        ) {
            ++tick;
            std::lock_guard< decltype(mutex) > lock(mutex);
            for (const auto system: systems) {
                system->Update(components, tick);
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
    impl_->AddMonster(6, 2);
    impl_->AddMonster(1, 7);
    for (int y = 0; y <= 8; ++y) {
        for (int x = 0; x <= 10; ++x) {
            if (
                ((x == 5) && (y == 5))
                || ((x == 6) && (y == 5))
                || ((x == 5) && (y == 6))
            ) {
                impl_->AddWall(x, y);
            } else {
                impl_->AddFloor(x, y);
            }
        }
    }
    impl_->SetWebSocketDelegates();
    impl_->worker = std::thread(&Impl::Worker, impl_.get());
}
