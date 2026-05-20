/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * MASTERs Class 25028 LNX2 - Lab 4
 * This application creates a graphical UI that connects to a server and streams video.
 */

#include <memory>

#include <egt/themes/midnight.h>   // Midnight theme for UI styling
#include <egt/ui>                  // Main EGT UI elements
#include <egt/asio.hpp>           // ASIO networking support via EGT
#include <iostream>               // Standard I/O for logging

using egt::asio::ip::tcp;

// Server details for messaging and video streaming
#define SERVER_ADDRESS      "10.0.0.10"
#define SERVER_MSG_PORT     "8000"
#define SERVER_VIDEO_PORT   "5000"  // Used in the GStreamer string

int main(int argc, char** argv)
{
    // Initialize the EGT application
    egt::Application app(argc, argv);

    // Create the main window (TopWindow) with specified size and pixel format
    egt::TopWindow win(egt::Size(720, 1280), egt::PixelFormat::argb8888);

    // Apply the "Midnight" theme globally
    egt::global_theme(std::make_unique<egt::MidnightTheme>());

    // Define a custom font for use in UI elements
    auto customFont = egt::Font("Sans Serif", 24, egt::Font::Weight::normal, egt::Font::Slant::normal);

    // Create and position a VideoWindow to receive video via GStreamer (requires hardware overlay)
    egt::VideoWindow player(egt::Size(320, 240), egt::PixelFormat::yuv420, egt::WindowHint::heo_overlay);
    player.move(egt::Point(200, 900));
    win.add(player);

    // Create the "Start Streaming" button and center it horizontally at the bottom
    auto streamButton = std::make_shared<egt::Button>("Start Streaming");
    streamButton->align(egt::AlignFlag::bottom | egt::AlignFlag::center_horizontal);
    streamButton->font(customFont);
    win.add(streamButton);

    // Create a SpinProgress (circular progress bar) with values from 0 to 100
    auto spinProgress = std::make_shared<egt::SpinProgress>(egt::Rect(160, 30, 400, 400), 0, 100, 0);
    spinProgress->font(egt::Font("Sans Serif", 48, egt::Font::Weight::normal, egt::Font::Slant::normal));
    spinProgress->show_label(true);
    win.add(spinProgress);

    // Create the "Open" button for unlocking, with visual styling
    auto button = std::make_shared<egt::Button>("Open", egt::Rect(260, 500, 200, 120));
    button->border(2);
    button->border_radius(12);
    button->font(customFont);
    win.add(button);

    // Create a label to display status messages from the server
    auto lbl = std::make_shared<egt::Label>("Press the button to unlock", egt::Rect(60, 680, 600, 100));
    lbl->border(1);
    lbl->text_align(egt::AlignFlag::center);
    lbl->font(customFont);
    win.add(lbl);

    // Prepare networking objects
    auto& io_context = app.event().io();                  // EGT event loop's I/O context
    auto socket = std::make_shared<tcp::socket>(io_context);
    auto resolver = std::make_shared<tcp::resolver>(io_context);

    // Lambda for recursively reading server messages asynchronously
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

                // Update the label in the UI thread
                egt::asio::post(egt::Application::instance().event().io(), [lbl, message]() {
                    lbl->text(message);
                });

                // Continue reading further messages
                start_reading();
            }
            else
            {
                std::cerr << "Read error: " << ec.message() << std::endl;
                std::exit(1);
            }
        });
    };

    // Resolve server address and connect on startup
    resolver->async_resolve(SERVER_ADDRESS, SERVER_MSG_PORT,
        [socket, start_reading](const egt::asio::error_code& ec, tcp::resolver::results_type endpoints)
    {
        if (!ec)
        {
            egt::asio::async_connect(*socket, endpoints,
                [socket, start_reading](const egt::asio::error_code& ec, const tcp::endpoint&)
            {
                if (!ec)
                {
                    std::cout << "Connected to server" << std::endl;
                    start_reading(); // Begin reading messages after connection
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

    // Timer used to animate the progress circle and send "Lock" after it completes
    egt::v1::PeriodicTimer animateTimer(std::chrono::milliseconds(100));
    animateTimer.on_timeout([&spinProgress, &animateTimer, socket]()
    {
        if (spinProgress->value() > 0)
        {
            spinProgress->value(spinProgress->value() - 5);  // Decrease value
        }
        else
        {
            animateTimer.cancel();  // Stop the animation when done

            // Send "Lock" command to server
            std::string data = "Lock";
            egt::asio::async_write(*socket, egt::asio::buffer(data),
                [socket](const egt::asio::error_code& ec, std::size_t bytes_transferred)
            {
                if (!ec)
                {
                    std::cout << "Lock message sent: " << bytes_transferred << " bytes" << std::endl;
                }
                else
                {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
            });
        }
    });

    // Action when "Open" button is clicked
    button->on_click([&, socket](egt::Event&)
    {
        animateTimer.cancel();          // Cancel any ongoing animation
        spinProgress->value(100);       // Reset progress to full
        animateTimer.start();           // Start the progress animation

        // Send "Unlock" command to server
        std::string data = "Unlock";
        egt::asio::async_write(*socket, egt::asio::buffer(data),
            [socket](const egt::asio::error_code& ec, std::size_t bytes_transferred)
        {
            if (!ec)
            {
                std::cout << "Unlock message sent: " << bytes_transferred << " bytes" << std::endl;
            }
            else
            {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
        });
    });

    // Start the GStreamer video stream on button click
    streamButton->on_click([&, socket](egt::Event&)
    {
        std::cout << "Starting Streaming" << std::endl;

        // GStreamer pipeline to decode JPEG stream over UDP into the video window
        player.gst_custom_pipeline("THIS NEEDS REPLACING WITH THE CORRECT PIPELINE");

        streamButton->hide();  // Hide the button after click
        player.show();         // Show video window
        player.play();         // Start playing stream

        // send a request to the server to stream the video over UDP
        // Just like the Lock and Unlock Commands add a call to send the message Play to the server

    });

    // Display the window and start the main loop
    win.show();
    win.layout();
    app.run();

    // Clean up: shutdown and close the socket connection
    egt::asio::error_code ec;
    socket->shutdown(tcp::socket::shutdown_both, ec);
    socket->close();
}
