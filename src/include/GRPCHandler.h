#pragma once

#include <RPCHandler.h>
#include <utility>
#include <thread>

#include <grpcpp/grpcpp.h>

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/log/initialize.h>
#include <absl/strings/str_format.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpc/grpc.h>
#include <grpcpp/impl/service_type.h>

#include <mandelbrot.grpc.pb.h>

//forward declarations
namespace grpc {
    class CallbackServerContext;
    class Server;
    class ServerBuilder;
    class ServerUnaryReactor;
    class Status;
    class ServerContext;
};

namespace mandelbrot {
    class Greeter;
    class HelloReply;
    class HelloRequest;
};

class GreeterServiceImpl final : public ::mandelbrot::Greeter::Service {

public:
    GreeterServiceImpl(MandelbrotWidget& widget) : m_widget(widget) {}

private:
    ::grpc::Status SayHello(::grpc::ServerContext* context, const ::mandelbrot::HelloRequest* request,
        ::mandelbrot::HelloReply* reply) override;

    ::grpc::Status RequestBitmap(::grpc::ServerContext* context, const ::mandelbrot::HelloRequest* request, ::mandelbrot::HelloBitmap* response) override;

    ::grpc::Status RequestBitmapNotifications(::grpc::ServerContext* context, const ::mandelbrot::BitmapUpdateRequest* request, ::mandelbrot::BitmapUpdateRequestReply* response) override;

    MandelbrotWidget& m_widget;
};

class GRPCHandler : public RPCHandler
{
public:
	explicit GRPCHandler(uint16_t port, MandelbrotWidget& widget);
	~GRPCHandler() override = default;

	void StartRPCService() override;
    void ShutdownService() override;

    //messaging from server to clients
    void NotifyNewBitmapAvailable();

private:

    void RunGRPCServer(uint16_t port, MandelbrotWidget& widget);
;
    std::unique_ptr<::grpc::Server> m_server;
    std::unique_ptr<std::jthread> m_serverThread;

    static constexpr char kServerCertPath[] = "../credentials/localhost.crt";
    static constexpr char kServerKeyPath[] = "../credentials/localhost.key";
};
