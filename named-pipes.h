#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <string>

#pragma pack(4)
struct Packet
{
    uint64_t descriptor;
    uint64_t buffer;  
};
#pragma pack()

class FIFOInterface 
{
private:
    int read_file;
    int write_file;
    Packet packet;
    bool write_ready;
    bool server_client;
public:
    FIFOInterface(const std::string& file, bool server);
    ~FIFOInterface();
    bool read();
    void write(const Packet& packet);
    Packet get_packet();
};

#ifdef NAMED_PIPES_IMPL
FIFOInterface::FIFOInterface(const std::string& file, bool server)
{
    if (server) mkfifo((file + "sread").c_str(), S_IFIFO|0640);
    if (server) mkfifo((file + "swrite").c_str(), S_IFIFO|0640);
    read_file = open((file + (server ? "sread" : "swrite")).c_str(), O_RDWR | O_NONBLOCK);
    write_file = open((file + (server ? "swrite" : "sread")).c_str(), O_RDWR | O_NONBLOCK);
    server_client = server;
    write_ready = server;
}    

FIFOInterface::~FIFOInterface()
{
    close(read_file);
    close(write_file);
}

bool FIFOInterface::read()
{
    size_t len = ::read(read_file, &packet, sizeof(Packet));
    if (len != sizeof(Packet))
    {
        return false;
    }

    return true;
}

void FIFOInterface::write(const Packet& packet)
{
    if (!write_ready) return;
    ::write(write_file, (const char*) &packet, sizeof(Packet));
    write_ready = false;
}

Packet FIFOInterface::get_packet()
{
    return packet;
}
#endif
