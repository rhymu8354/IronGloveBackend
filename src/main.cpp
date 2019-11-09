/**
 * @file main.cpp
 *
 * This module holds the main() function, which is the entrypoint
 * to the program.
 *
 * © 2018 by Richard Walters
 */

#include "game.hpp"
#include "TimeKeeper.hpp"

#include <functional>
#include <Http/Server.hpp>
#include <HttpNetworkTransport/HttpServerNetworkTransport.hpp>
#include <Json/Value.hpp>
#include <memory>
#include <set>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <StringExtensions/StringExtensions.hpp>
#include <SystemAbstractions/DiagnosticsSender.hpp>
#include <SystemAbstractions/DiagnosticsStreamReporter.hpp>
#include <SystemAbstractions/NetworkConnection.hpp>
#include <thread>
#include <vector>
#include <WebSockets/WebSocket.hpp>

namespace {

    /**
     * This flag indicates whether or not the server should shut down.
     */
    bool SHUTDOWN = false;

    /**
     * This function is set up to be called when the SIGINT signal is
     * received by the program.  It just sets the "SHUTDOWN" flag
     * and relies on the program to be polling the flag to detect
     * when it's been set.
     *
     * @param[in] sig
     *     This is the signal for which this function was called.
     */
    void InterruptHandler(int) {
        SHUTDOWN = true;
    }

    using WebSocketDelegate = std::function<
        void(
            const std::string& id,
            std::shared_ptr< WebSockets::WebSocket > ws
        )
    >;

    bool SetUpWebServer(
        Http::Server& webServer,
        std::shared_ptr< TimeKeeper > timeKeeper,
        SystemAbstractions::DiagnosticsSender::DiagnosticMessageDelegate diagnosticMessageDelegate,
        WebSocketDelegate webSocketDelegate
    ) {
        auto transport = std::make_shared< HttpNetworkTransport::HttpServerNetworkTransport >();
        transport->SubscribeToDiagnostics(diagnosticMessageDelegate);
        webServer.SubscribeToDiagnostics(diagnosticMessageDelegate);
        Http::Server::MobilizationDependencies httpDeps;
        httpDeps.timeKeeper = timeKeeper;
        httpDeps.transport = transport;
        webServer.SetConfigurationItem("Port", "8080");
        webServer.RegisterResource(
            {},
            [
                diagnosticMessageDelegate,
                webSocketDelegate
            ](
                const Http::Request& request,
                std::shared_ptr< Http::Connection > connection,
                const std::string& trailer
            ){
                Http::Response response;
                const auto ws = std::make_shared< WebSockets::WebSocket >();
                (void)ws->SubscribeToDiagnostics(diagnosticMessageDelegate);
                if (ws->OpenAsServer(connection, request, response, trailer)) {
                    webSocketDelegate(connection->GetPeerId(), ws);
                } else {
                    response.statusCode = 404;
                    response.reasonPhrase = "Not Found";
                    response.headers.SetHeader("Content-Type", "text/plain");
                    response.body = "FeelsBadMan\r\n";
                }
                return response;
            }
        );
        if (!webServer.Mobilize(httpDeps)) {
            return false;
        }
        return true;
    }

    void TearDownWebServer(Http::Server& webServer) {
        webServer.Demobilize();
    }

    /**
     * This function is called from the main function, once the server
     * is up and running.  It returns once the program has been signaled
     * to end.
     */
    void WaitForShutDown() {
        while (!SHUTDOWN) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }

}

/**
 * This function is the entrypoint of the program.
 * It just sets up the server and then waits for
 * the SIGINT signal to know when the server should
 * be shut down and program terminated.
 *
 * @param[in] argc
 *     This is the number of command-line arguments given to the program.
 *
 * @param[in] argv
 *     This is the array of command-line arguments given to the program.
 */
int main(int argc, char* argv[]) {
#ifdef _WIN32
    //_crtBreakAlloc = 18;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif /* _WIN32 */
    const auto previousInterruptHandler = signal(SIGINT, InterruptHandler);
    (void)setbuf(stdout, NULL);
    auto diagnosticsPublisher = SystemAbstractions::DiagnosticsStreamReporter(stdout, stderr);
    const auto timeKeeper = std::make_shared< TimeKeeper >();
    const auto webServer = std::make_shared< Http::Server >();
    std::set< std::shared_ptr< Game > > games;
    const auto webSocketDelegate = [
        &games,
        timeKeeper,
        diagnosticsPublisher
    ](
        const std::string& id,
        std::shared_ptr< WebSockets::WebSocket > ws
    ){
        const auto game = std::make_shared< Game >(id);
        std::weak_ptr< Game > gameWeak(game);
        const auto completeDelegate = [&games, gameWeak]{
            const auto game = gameWeak.lock();
            if (game == nullptr) {
                return;
            }
            (void)games.erase(game);
        };
        (void)games.insert(game);
        game->Start(ws, timeKeeper, diagnosticsPublisher, completeDelegate);
    };
    if (
        !SetUpWebServer(
            *webServer,
            timeKeeper,
            diagnosticsPublisher,
            webSocketDelegate
        )
    ) {
        return EXIT_FAILURE;
    }
    diagnosticsPublisher("Server", 3, "Server up and running.");
    WaitForShutDown();
    (void)signal(SIGINT, previousInterruptHandler);
    diagnosticsPublisher("Server", 3, "Shutting Down...");
    TearDownWebServer(*webServer);
    diagnosticsPublisher("Server", 3, "Exiting...");
    return EXIT_SUCCESS;
}
