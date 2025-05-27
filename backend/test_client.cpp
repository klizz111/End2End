#include "core.hpp"

int main() {
    Core client(ServerMode::CLIENT, 1024);
    client.SetDestination("localhost", 8848);
    client.start();
    client.ConnectionTest();
    client.GetServerPublicKey();
    cin.get();
}