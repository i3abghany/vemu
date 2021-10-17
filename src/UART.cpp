#include <UART.h>

UART::UART()
{
    uart_mem = std::array<uint8_t, UART_SIZE> {};

    {
        std::lock_guard<std::mutex> lock(mu);

        // set 0th bit, indicating that the transmit-holding reg is empty
        uart_mem[UART_LSR - UART_BASE] |= UART_LSR_TX;
    }

    std::thread read_thread([this]() {
        unsigned char c;

        while (true) {
            std::cin >> std::noskipws >> c;

            /* TODO: Use a condvar instead of spinning. */
            while ((uart_mem[UART_LSR - UART_BASE] & UART_LSR_RX) == 1)
                ;

            uart_mem[UART_RHR - UART_BASE] = c;
            uart_mem[UART_LSR - UART_BASE] |= UART_LSR_RX;
        }
    });

    read_thread.detach();
}

std::pair<uint64_t, ReturnException> UART::load(uint64_t addr, size_t sz)
{
    std::pair<uint64_t, ReturnException> res;
    res.second = ReturnException::NormalExecutionReturn;

    switch (sz) {
        case 8:
            res.first = load8(addr);
            break;
        default:
            res.second = ReturnException::LoadAccessFault;
            break;
    }

    return res;
}


ReturnException UART::store(uint64_t addr, uint64_t value, size_t sz)
{
    ReturnException res;
    res = ReturnException::NormalExecutionReturn;

    switch (sz) {
        case 8:
            store8(addr, value);
            break;
        default:
            res = ReturnException::StoreAMOAccessFault;
            break;
    }

    return res;
}

uint64_t UART::load8(uint64_t addr)
{
    std::lock_guard<std::mutex> lock(mu);

    if(addr == UART_RHR) {
        uart_mem[UART_LSR - UART_BASE] &= static_cast<uint8_t>(~(UART_LSR_RX));
    }

    return uart_mem[addr - UART_BASE];
}

void UART::store8(uint64_t addr, uint64_t value)
{
    std::lock_guard<std::mutex> lock(mu);

    switch (addr) {
        case UART_THR:
            std::cout << static_cast<char>(value) << std::flush;
            return;
        default:
            uart_mem[addr - UART_BASE] = static_cast<uint8_t>(value & 0xFF);
    }
}
