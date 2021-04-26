#include "../include/CLIC.h"

CLIC::CLIC()
{
    mtime = 0;
    mtimecmp = 0;
}

uint64_t CLIC::load64(uint64_t addr)
{
    switch (addr) {
        case CLIC_MTIMECMP:
            return mtimecmp;
        case CLIC_MTIME:
            return mtime;
        default:
            break;
    }

    return 0;
}

void CLIC::store64(uint64_t addr, uint64_t data)
{
    switch (addr) {
        case CLIC_MTIMECMP:
            mtimecmp = data;
            break;
        case CLIC_MTIME:
            mtime = data;
            break;
        default:
            break;
    }
}

std::pair<uint64_t, ReturnException> CLIC::load(uint64_t addr, size_t sz)
{
    std::pair<uint64_t, ReturnException> res;
    res.second = ReturnException::NormalExecutionReturn;

    switch (sz) {
        case 64:
            res.first = load64(addr);
            break;
        default:
            res.second = ReturnException::LoadAccessFault;
            break;
    }

    return res;
}

ReturnException CLIC::store(uint64_t addr, uint64_t value, size_t sz)
{
    ReturnException res;
    res = ReturnException::NormalExecutionReturn;

    switch (sz) {
        case 64:
            store64(addr, value);
            break;
        default:
            res = ReturnException::StoreAMOAccessFault;
            break;
    }

    return res;
}
