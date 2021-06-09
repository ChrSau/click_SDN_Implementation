
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>

#include "TCPClient.hh"
#include "Packet.hh"
#include "../OFHelper.hh"
#include "OFResponder_Test.hh"
#include "OFPipeline.hh"

TCPClient tcp;
std::queue<myPacket *> socketQueue;
std::mutex queueLock;
bool readThread = false;
OFPipeline *pipeline;
OFResponder *responder;
bool run = true;

void readDataFromQueue()
{
    while (readThread)
    {
        if (socketQueue.size() > 0)
        {
            for (int i = 0; socketQueue.size() > 0; i++)
            {

                myPacket *request = new myPacket(socketQueue.front()->data(), socketQueue.front()->length());

                myPacket *response = responder->simple_action(request);

                if (response != nullptr && response != NULL)
                {
                    if (response->length() > 0)
                    {
                        if(!tcp.Send(response->data(), response->length())) {
                            std::cerr << "tcp.send schlug fehl\n" << std::endl;
                        }
                    }
                }
                delete response;
                response = nullptr;

                queueLock.lock();
                delete socketQueue.front();
                socketQueue.pop();
                queueLock.unlock();

                delete (request);

                request = nullptr;
            }
        }
        // usleep(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    // std::cout << "Ende" << std::endl;
}

void readDataFromSocket()
{
    readThread = true;
    uint8_t buffer[4096];
    uint32_t len = 0;
    while (run)
    {
        std::memset(buffer, 0, sizeof(buffer));

        if (!tcp.receive(buffer, 255, &len))
            break;

        if (len > 0)
        {
            // OF::Functions::printHex(buffer, len, "Response: ");
            myPacket *packet = new myPacket();
            packet->make(buffer, len);

            queueLock.lock();
            socketQueue.push(packet);
            queueLock.unlock();
        }
        // usleep(10);
    }
    readThread = false;
}

int main(int argc, char const *argv[])
{
    pipeline = new OFPipeline(&tcp, 0);
    responder = new OFResponder(pipeline, &tcp, &socketQueue, &queueLock);

    if (!tcp.setup("143.93.156.156", 6653))
    {
        std::cerr << "Fehler beim aufbau der Verbindung.\n";
    }
    else
    {
        std::cout << "Connection established\n";
    }

    std::thread readFromSocket(readDataFromSocket);
    std::thread readFromQueue(readDataFromQueue);

    readFromQueue.detach();

    char message;
    std::cout << "Quit with q\n"
              << std::endl;
    std::cin >> message;

    if (message)
        run = false;

    readFromSocket.join();

    delete responder;
    delete pipeline;

    tcp.exit();

    for (size_t i = 0; i < socketQueue.size(); i++)
    {
        // socketQueue.front()->kill();
        delete socketQueue.front();
        socketQueue.pop();
    }

    return 0;
}
