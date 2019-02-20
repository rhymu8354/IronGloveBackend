#include "Components.hpp"
#include "game.hpp"
#include "Systems.hpp"

#include <future>
#include <Json/Value.hpp>
#include <math.h>
#include <mutex>
#include <string>
#include <SystemAbstractions/DiagnosticsSender.hpp>
#include <thread>

struct Game::Impl
    : public std::enable_shared_from_this< Game::Impl >
{
    std::shared_ptr< WebSockets::WebSocket > ws;
    std::shared_ptr< TimeKeeper > timeKeeper;
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
                input.fireReleased = true;
                if (!input.fireThisTick) {
                    input.fire = 0;
                }
            } else {
                const auto key = keyString[0];
                input.fireReleased = false;
                input.fireThisTick = true;
                input.fire = key;
            }
        } else if (message["type"] == "move") {
            const auto keyString = (std::string)message["key"];
            if (keyString.empty()) {
                input.moveReleased = true;
                if (!input.moveThisTick) {
                    input.move = 0;
                }
            } else {
                const auto key = keyString[0];
                input.moveReleased = false;
                input.moveThisTick = true;
                input.move = key;
            }
        } else if (message["type"] == "potion") {
            input.usePotion = true;
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
        const auto health = (Health*)components.CreateComponentOfType(Components::Type::Health, id);
        const auto hero = (Hero*)components.CreateComponentOfType(Components::Type::Hero, id);
        const auto input = (Input*)components.CreateComponentOfType(Components::Type::Input, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        collider->mask = 1;
        tile->name = "hero";
        tile->z = 2;
        position->x = x;
        position->y = y;
        health->hp = 100;
        hero->score = 0;
        hero->potions = 0;
    }

    void AddMonster(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto collider = (Collider*)components.CreateComponentOfType(Components::Type::Collider, id);
        const auto health = (Health*)components.CreateComponentOfType(Components::Type::Health, id);
        const auto monster = (Monster*)components.CreateComponentOfType(Components::Type::Monster, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        const auto reward = (Reward*)components.CreateComponentOfType(Components::Type::Reward, id);
        collider->mask = 2;
        tile->name = "monster";
        tile->z = 2;
        position->x = x;
        position->y = y;
        health->hp = 1;
        reward->score = 10;
    }

    void AddGenerator(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto collider = (Collider*)components.CreateComponentOfType(Components::Type::Collider, id);
        const auto generator = (Generator*)components.CreateComponentOfType(Components::Type::Generator, id);
        const auto health = (Health*)components.CreateComponentOfType(Components::Type::Health, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        const auto reward = (Reward*)components.CreateComponentOfType(Components::Type::Reward, id);
        collider->mask = ~0;
        tile->name = "bones";
        tile->z = 1;
        position->x = x;
        position->y = y;
        generator->spawnChance = 0.05;
        health->hp = 10;
        reward->score = 250;
    }

    void AddWall(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto collider = (Collider*)components.CreateComponentOfType(Components::Type::Collider, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        collider->mask = ~0;
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

    void AddTreasure(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto pickup = (Pickup*)components.CreateComponentOfType(Components::Type::Pickup, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "treasure";
        tile->z = 1;
        position->x = x;
        position->y = y;
        pickup->type = Pickup::Type::Treasure;
    }

    void AddFood(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto pickup = (Pickup*)components.CreateComponentOfType(Components::Type::Pickup, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "food";
        tile->z = 1;
        position->x = x;
        position->y = y;
        pickup->type = Pickup::Type::Food;
    }

    void AddPotion(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto pickup = (Pickup*)components.CreateComponentOfType(Components::Type::Pickup, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "potion";
        tile->z = 1;
        position->x = x;
        position->y = y;
        pickup->type = Pickup::Type::Potion;
    }

    void AddExit(unsigned int x, unsigned int y) {
        const auto id = components.CreateEntity();
        const auto pickup = (Pickup*)components.CreateComponentOfType(Components::Type::Pickup, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "exit";
        tile->z = 1;
        position->x = x;
        position->y = y;
        pickup->type = Pickup::Type::Exit;
    }

    void Worker() {
        diagnosticsSender.SendDiagnosticInformationString(
            3,
            "Worker started!"
        );
        auto workerToldToStop = stopWorker.get_future();
        size_t tick = 0;
        const double measurementIntervalSeconds = 3.0;
        const int ticksPerSecond = 10;
        const size_t measurementIntervalLoops = (size_t)round(measurementIntervalSeconds * ticksPerSecond);
        double minMeasurement, sumMeasurements, maxMeasurement;
        size_t numMeasurements = 0;
        while (
            workerToldToStop.wait_for(std::chrono::milliseconds(1000/ticksPerSecond))
            != std::future_status::ready
        ) {
            if (numMeasurements == 0) {
                minMeasurement = 0.0;
                sumMeasurements = 0.0;
                maxMeasurement = 0.0;
            }
            const auto start = timeKeeper->GetCurrentTime();
            ++tick;
            std::lock_guard< decltype(mutex) > lock(mutex);
            for (const auto system: systems) {
                system->Update(components, tick);
            }
            const auto finish = timeKeeper->GetCurrentTime();
            const auto measurement = (finish - start);
            if (numMeasurements == 0) {
                minMeasurement = measurement;
            } else {
                minMeasurement = std::min(minMeasurement, measurement);
            }
            sumMeasurements += measurement;
            maxMeasurement = std::max(maxMeasurement, measurement);
            if (++numMeasurements >= measurementIntervalLoops) {
                const auto avgMeasurement = sumMeasurements / numMeasurements;
                diagnosticsSender->SendDiagnosticInformationFormatted(
                    3,
                    "min=%lf avg=%lf max=%lf",
                    minMeasurement,
                    avgMeasurement,
                    maxMeasurement
                );
                numMeasurements = 0;
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
    std::shared_ptr< TimeKeeper > timeKeeper,
    SystemAbstractions::DiagnosticsSender::DiagnosticMessageDelegate diagnosticMessageDelegate,
    CompleteDelegate completeDelegate
) {
    impl_->diagnosticsSender.SubscribeToDiagnostics(diagnosticMessageDelegate);
    impl_->diagnosticsSender.SendDiagnosticInformationString(
        3,
        "Try this level now!"
    );
    impl_->ws = ws;
    impl_->timeKeeper = timeKeeper;
    impl_->completeDelegate = completeDelegate;
    impl_->systems = Systems(ws);
    impl_->AddPlayer(1, 1);
    impl_->AddMonster(6, 2);
    impl_->AddMonster(1, 7);
    impl_->AddGenerator(8, 4);
    impl_->AddTreasure(2, 10);
    impl_->AddTreasure(3, 10);
    impl_->AddTreasure(3, 11);
    impl_->AddTreasure(8, 8);
    impl_->AddTreasure(9, 8);
    impl_->AddFood(12, 9);
    impl_->AddPotion(6, 6);
    impl_->AddExit(13, 11);
    for (int y = 0; y <= 12; ++y) {
        for (int x = 0; x <= 14; ++x) {
            if (
                (
                    ((x == 5) && (y == 5))
                    || ((x == 6) && (y == 5))
                    || ((x == 5) && (y == 6))
                )
                || (x == 0)
                || (x == 14)
                || (y == 0)
                || (y == 12)
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
