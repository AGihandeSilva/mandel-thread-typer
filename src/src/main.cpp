/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mandelbrotwidget.h"

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QStyleFactory>
#include <QImage>
#if defined(MANDELBROT_ENABLE_HTTP_SERVER_CODE)
#include <HTTPServer.h>
#endif //MANDELBROT_ENABLE_HTTP_SERVER_CODE
#include <SynchronizedData.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <fstream>
#include <nlohmann/json.hpp>

#if defined(MANDEL_USE_GRPC)
#include <absl/flags/flag.h>
#include <GRPCHandler.h>
#endif

using namespace std::chrono_literals;

static void restrictToOneInstance(QSharedMemory& sharedSpace);

Q_DECLARE_METATYPE(InformationDisplay::renderState);

#if defined(MANDELBROT_ENABLE_HTTP_SERVER_CODE)

void HttpReceiverTask(std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData, std::shared_ptr<asio::io_context> ioc)
{
    if (!ioc)
    {
        std::cerr << "Error, no ioc context supplied " << std::endl;
        return;
    }
    try {
        auto const address = asio::ip::make_address("0.0.0.0");
        unsigned short port = 8080;

        std::make_shared<HTTPListener>(*ioc, tcp::endpoint{ address, port }, synchronizedData)->run();

        ioc->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Http server error: " << e.what() << std::endl;
    }
}

void HttpTransmitterTask(std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedInData,
    std::shared_ptr<SynchronizedData<std::string>> synchronizedOutData)
{
    do
    {
        if (synchronizedInData && !synchronizedInData->empty())
        {
            auto message = synchronizedInData->PopMessage();
            if (message.has_value())
            {
                const http::request<http::string_body> req = message.value();
                auto req_body = req.body();
                auto req_path = std::string(req.target());
                auto req_method = std::string(http::to_string(req.method()));

                nlohmann::json body = nlohmann::json::parse(req_body, nullptr, false);

                const std::string bodyString = body.is_discarded() ? "(None)" : body.dump(0);

                const std::string incomingMessage = req_method + (" : ") + req_path + " body: " + bodyString;
                //const std::string incomingMessage = "TODO";
                std::cout << "incoming message: " << incomingMessage << std::endl;

                std::string outgoingMessage = incomingMessage; //TODO

                if (synchronizedOutData)
                {
                    synchronizedOutData->PushMessage(outgoingMessage);
                }
            }
        }
        std::this_thread::sleep_for(20ms);
    } while (true);
}

#endif //MANDELBROT_ENABLE_HTTP_SERVER_CODE

#if defined(MANDEL_USE_GRPC)
ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");
#endif

//! [0]
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<InformationDisplay::renderState>("renderState");

    QApplication::setStyle(QStyleFactory::create("fusion"));

#if defined (MANDELBROT_ENABLE_HTTP_SERVER_CODE)

    auto synchronizedInData = std::make_shared<SynchronizedData<http::request<http::string_body>>>();

    std::vector<std::jthread> commsThreads;

    auto ioc = std::make_shared<asio::io_context>(1);

    commsThreads.push_back(std::jthread{ HttpReceiverTask, synchronizedInData, std::shared_ptr(ioc) });

    auto synchronizedOutData = std::make_shared<SynchronizedData<std::string>>();

    commsThreads.push_back(std::jthread{ HttpTransmitterTask, synchronizedInData, synchronizedOutData/*, std::shared_ptr(ioc) */});
#endif //MANDELBROT_ENABLE_HTTP_SERVER_CODE

    MandelbrotWidget widget;
    widget.show();

#if defined(MANDEL_USE_GRPC)
    //build and start the gRPC service
    GRPCHandler serviceHandler(absl::GetFlag(FLAGS_port), std::ref(widget));
    serviceHandler.StartRPCService();
#endif

    int result = QApplication::exec();

#if defined (MANDELBROT_ENABLE_HTTP_SERVER_CODE)
    bool allStopped = true;
    for (auto& t : commsThreads)
    {
        allStopped &= t.request_stop();
    }
    std::cout << "All comms threads stopped: " << allStopped << std::endl;
#endif //MANDELBROT_ENABLE_HTTP_SERVER_CODE

#if defined(MANDEL_USE_GRPC)
    serviceHandler.ShutdownService();
#endif

    exit(0);

    return result;
}

void restrictToOneInstance(QSharedMemory& sharedSpace)
{
    const QString spaceName = QSysInfo::machineHostName() + "::" + "MandelbrotSpace";
    sharedSpace.setKey(spaceName);

    const int sharedSize = 16;
    if (!sharedSpace.create(sharedSize, QSharedMemory::ReadWrite))
    {
        auto NoDuplicateWarning = new QMessageBox();
        NoDuplicateWarning->setText("Only one renderer may be run at a time");
        NoDuplicateWarning->exec();
        exit(0);
    }
}
//! [0]
