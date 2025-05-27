#include "core.hpp"

int main() {
    Core core(ServerMode::SERVER, 1024);

    core.start();

    cin.get();
}