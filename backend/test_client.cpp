#include "core.hpp"

int main() {
    Core client(ServerMode::CLIENT, 512);
    client.SetDestination("localhost", 8848);
    client.start();
    client.ConnectionTest();
    client.GetServerPublicKey();
    client.WhereIsMyServer();
    cin.get();
}