#include <GRPCHandler.h>

#include "mandelbrotwidget.h"

#include <chrono>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerUnaryReactor;
using grpc::Status;
using grpc::ServerContext;
using mandelbrot::Greeter;
using mandelbrot::HelloReply;
using mandelbrot::HelloRequest;

Status GreeterServiceImpl::SayHello(ServerContext* context, const HelloRequest* request,
    HelloReply* reply)
 {
    std::string prefix("Hello ");
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    reply->set_message(prefix + request->name() + ", the time is now: " + std::ctime(&nowTime));
    return Status::OK;
}

Status GreeterServiceImpl::RequestBitmap(::grpc::ServerContext* context, const ::mandelbrot::HelloRequest* request, ::mandelbrot::HelloBitmap* response)
{

    QPixmap pixmap = m_widget.getPixmap();
    [[maybe_unused]] QSize size = pixmap.size();
    QImage bitmap = pixmap.toImage();

    [[maybe_unused]] const bool isNull = bitmap.isNull();
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    [[maybe_unused]] const bool savedOK = bitmap.save(&buffer, "PNG");
    [[maybe_unused]] const qsizetype bytesSize = bytes.size();
    response->set_pixeldata(bytes, bytesSize);

    return Status::OK;
}

Status GreeterServiceImpl::RequestBitmapNotifications(::grpc::ServerContext* context, const ::mandelbrot::BitmapUpdateRequest* request, ::mandelbrot::BitmapUpdateRequestReply* response)
{
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    response->set_message(std::string("Bitmap Update Notification") + std::string(std::ctime(&nowTime)));
    response->set_status(0);

    return Status::OK;
}

GRPCHandler::GRPCHandler(uint16_t port, MandelbrotWidget& widget) :
    RPCHandler(port, widget) {}

void GRPCHandler::StartRPCService()
{
    absl::InitializeLog();
    m_serverThread = std::make_unique<std::jthread>( &GRPCHandler::RunGRPCServer, this, m_port, std::ref(m_widget) );
}

void GRPCHandler::RunGRPCServer(uint16_t port, MandelbrotWidget& widget)
{
    std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

    GreeterServiceImpl service{ widget };

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    // Load SSL credentials and build a SSL credential options
    grpc::SslServerCredentialsOptions::PemKeyCertPair key_cert_pair = {
        LoadStringFromFile(kServerKeyPath), LoadStringFromFile(kServerCertPath) };
    grpc::SslServerCredentialsOptions ssl_options;
    ssl_options.pem_key_cert_pairs.emplace_back(key_cert_pair);

    // Listen on the given address with SSL credentials
    builder.AddListeningPort(server_address,
        grpc::SslServerCredentials(ssl_options));

    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);

    // Finally assemble the server.
    m_server = builder.BuildAndStart();

    if (m_server)
    {
        std::cout << "Server listening on " << server_address << std::endl;

        // Wait for the server to shutdown. Note that some other thread must be
        // responsible for shutting down the server for this call to ever return.
        m_server->Wait();
    }
    else
    {
        std::cerr << "gRPC server failed to start at " << server_address << std::endl;
    }
}

void GRPCHandler::NotifyNewBitmapAvailable()
{
//TODO
}

void GRPCHandler::ShutdownService()
{
    if (m_server)
    {
        m_server->Shutdown();
    }
}
