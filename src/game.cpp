#include "game.hpp"

#include <future>
#include <Json/Value.hpp>
#include <string>
#include <SystemAbstractions/DiagnosticsSender.hpp>
#include <thread>

struct Game::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
    CompleteDelegate completeDelegate;
    SystemAbstractions::DiagnosticsSender diagnosticsSender;
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
            static int counter = 0;
            if (++counter % 2 == 0) {
                ws->SendText(
                    Json::Object({
                        {"type", "render"},
                        {"sprites", Json::Array({
                            Json::Object({
                                {"id", 1},
                                {"texture", "hero"},
                                {"x", 0},
                                {"y", 0},
                            }),
                            Json::Object({
                                {"id", 2},
                                {"texture", "monster"},
                                {"x", 1},
                                {"y", 2},
                            }),
                            Json::Object({
                                {"id", 3},
                                {"texture", "monster"},
                                {"x", 1},
                                {"y", 0},
                            }),
                            Json::Object({
                                {"id", 4},
                                {"texture", "monster"},
                                {"x", 0},
                                {"y", 1},
                            }),
                        })},
                    }).ToEncoding()
                );
            } else {
                ws->SendText(
                    Json::Object({
                        {"type", "render"},
                        {"sprites", Json::Array({
                            Json::Object({
                                {"id", 1},
                                {"texture", "hero"},
                                {"x", 0},
                                {"y", 0},
                            }),
                            Json::Object({
                                {"id", 3},
                                {"texture", "monster"},
                                {"x", 1},
                                {"y", 0},
                            }),
                            Json::Object({
                                {"id", 4},
                                {"texture", "monster"},
                                {"x", 0},
                                {"y", 1},
                            }),
                        })},
                    }).ToEncoding()
                );
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
    WebSockets::WebSocket::Delegates delegates;
    std::weak_ptr< Impl > implWeak(impl_);
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
    impl_->worker = std::thread(&Impl::Worker, impl_.get());
}
