#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

using i8    = std::int8_t;
using u8    = std::uint8_t;
using i32   = std::int32_t;
using u32   = std::uint32_t;
using i64   = std::int16_t;
using u16   = std::uint16_t;
using usize = std::size_t;

static bool g_Running   = true;
static bool g_Connected = false;

static void SignalHandler(i32);

// Function to set the terminal into non-canonical mode (character-by-character input)
void SetNonCanonicalMode(const bool enable)
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty); // Get current terminal attributes

    if (enable)
    {
        tty.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    }
    else
    {
        tty.c_lflag |= (ICANON | ECHO); // Re-enable canonical mode and echo
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &tty); // Apply the new terminal settings immediately
}

void Connect(const std::string_view ipv4, const u16 port)
{
    constexpr auto BUFFER_SIZE = 1024;

    i32                sock = 0;
    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        std::cerr << "Socket creation error" << std::endl;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);

    // Convert IPv4 address from text to binary form
    inet_pton(AF_INET, ipv4.data(), &serv_addr.sin_addr);

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        g_Connected = false;
        std::cerr << "Connection Failed" << std::endl;
    }
    else
    {
        g_Connected = true;
        std::system("clear");
        SetNonCanonicalMode(true);
    }

    dup2(sock, STDIN_FILENO);
    dup2(sock, STDOUT_FILENO);

    // Use select() to handle both input from the terminal and server output
    fd_set fds;
    char   buffer[BUFFER_SIZE]{ 0 };
    while (g_Connected)
    {
        continue;
        /*
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds); // Monitor stdin
        FD_SET(sock, &fds);         // Monitor socket

        // Select returns the number of ready descriptors
        const i32 max_fd   = std::max(STDIN_FILENO, sock);
        const i32 activity = select(max_fd + 1, &fds, nullptr, nullptr, nullptr);

        if (activity < 0)
        {
            std::cerr << "Select error" << std::endl;
            break;
        }

        // Check if there's input from the terminal
        if (FD_ISSET(STDIN_FILENO, &fds))
        {
            const auto read_bytes = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (read_bytes > 0)
                send(sock, buffer, read_bytes, 0);
        }

        // Check if there's output from the server
        if (FD_ISSET(sock, &fds))
        {
            const auto read_bytes = read(sock, buffer, BUFFER_SIZE);
            if (read_bytes > 0)
                write(STDOUT_FILENO, buffer, read_bytes);
        }
        */
    }

    SetNonCanonicalMode(false);

    close(sock);
}

int main()
{
    struct sigaction sigact;
    sigact.sa_handler = SignalHandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, (struct sigaction*)nullptr);

    while (g_Running)
    {
        std::string ipv4;
        short       port;

        std::cout << ">>> ";
        std::string connect_str;
        std::getline(std::cin, connect_str);

        if (!connect_str.empty())
        {
            if (connect_str.starts_with("connect "))
            {
                std::istringstream ss{ connect_str.substr(sizeof("connect"), connect_str.size() - sizeof("connect")) };
                ss >> ipv4;
                ss >> port;
                Connect(ipv4, port);
            }
        }
    }

    return 0;
}

void SignalHandler(i32 signal)
{
    if (signal == SIGINT)
    {
        if (g_Connected)
            g_Connected = false;
        else
            std::exit(0);
    }
    else if (signal == SIGSTOP)
    {
        std::exit(0);
        g_Connected = false;
        g_Running   = false;
    }
}
