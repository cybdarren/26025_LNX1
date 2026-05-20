
#include <memory>

#include <egt/ui>
#include <egt/asio.hpp>
#include <egt/uiloader.h>
#include <iostream>

using egt::asio::ip::tcp;

int main(int argc, char** argv)
{
    egt::Application app(argc, argv);
 
    egt::experimental::UiLoader loader;
    auto window = std::static_pointer_cast<egt::Window>(loader.load("file:lab3_ui.xml"));
    if (!window)
    {
        std::cerr << "Error: Unable to load UI from lab3_ui.xml\n";
        return 1;
    }
 
    auto spinProgress = window->find_child<egt::SpinProgress>("SpinProgress1");
    auto button = window->find_child<egt::Button>("Button1");
    auto lbl = window->find_child<egt::Label>("lblMessage");

    if (!spinProgress || !button || !lbl)
    {
        std::cerr << "Error: UI elements not found\n";
        return 1;
    }
    
    auto& io_context = app.event().io();
    auto socket = std::make_shared<tcp::socket>(io_context);
    auto resolver = std::make_shared<tcp::resolver>(io_context);
  
    // a lambda function to read from the socket
    std::function<void()> start_reading;
    start_reading = [socket, lbl, &start_reading]()
    {
        auto buffer = std::make_shared<std::vector<char>>(1024);
    
        socket->async_read_some(egt::asio::buffer(*buffer), 
            [socket, lbl, buffer, &start_reading](const egt::asio::error_code& ec, std::size_t length)
        {
            if (!ec)
            {
                std::string message(buffer->data(), length);
                std::cout << "Received from server: " << message << std::endl;
    
                // Post update to UI thread
                egt::asio::post(egt::Application::instance().event().io(), [lbl, message]() {
                    lbl->text(message);
                });
    
                // Recursively call to keep reading
                start_reading();
            }
            else
            {
                std::cerr << "Read error: " << ec.message() << std::endl;
                std::exit(1);
            }
        });
    };
    
    // connect to the server at startup
    resolver->async_resolve("10.0.0.10", "8000",
        [socket, start_reading](const egt::asio::error_code& ec, tcp::resolver::results_type endpoints)
    {
        if (!ec)
        {
            egt::asio::async_connect(*socket, endpoints,
                [socket, start_reading] (const egt::asio::error_code& ec, const tcp::endpoint&)
            {
                if (!ec)
                {
                    std::cout << "Connected to server" << std::endl;
                    start_reading();
                }    
                else
                {
                    std::cerr << "Connect error: " << ec.message() << std::endl;
                }
            });
        }
        else
        {
            std::cerr << "Resolve error: " << ec.message() << std::endl;    
        }
    });

    egt::v1::PeriodicTimer animateTimer(std::chrono::milliseconds(100));
    animateTimer.on_timeout([&spinProgress, &animateTimer, socket] ()
    {
        if (spinProgress->value() > 0)
        {
            spinProgress->value(spinProgress->value() - 5);
        }
        else
        {
            animateTimer.cancel();

            // Add code to send a message when the timer is completed

        }
    });
    
    button->on_click([&, socket] (egt::Event&)
    {
        animateTimer.cancel();
        spinProgress->value(100);
        animateTimer.start();

        // Add code to send a message when the timer is started

    });

    window->show();
    app.run();

    // close the connection
    egt::asio::error_code ec;
    socket->shutdown(tcp::socket::shutdown_both, ec);
    socket->close();
}
