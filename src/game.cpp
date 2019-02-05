#include "game.hpp"

#include <string>
#include <SystemAbstractions/DiagnosticsSender.hpp>

struct Game::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
    CompleteDelegate completeDelegate;
    SystemAbstractions::DiagnosticsSender diagnosticsSender;


    explicit Impl(const std::string& id)
        : diagnosticsSender(id)
    {
    }

    void OnWebSocketClosed() {
        diagnosticsSender.SendDiagnosticInformationString(
            3,
            "Goodbye!"
        );
        completeDelegate();
    }

    void OnWebSocketText(const std::string& data) {
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
}
