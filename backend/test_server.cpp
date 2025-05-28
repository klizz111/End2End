#include "core.hpp"

int main() {
    Core core(ServerMode::SERVER, 512);

    core.start();

    cin.get();
}