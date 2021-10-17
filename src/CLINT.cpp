#include <CLINT.h>

CLINT::CLINT()
{
    mtime = 0;
    mtimecmp = 0;
}

uint64_t CLINT::load64(uint64_t addr) const
{
    switch (addr) {
        case CLINT_MTIMECMP:
            return mtimecmp;
        case CLINT_MTIME:
            return mtime;
        default:
            break;
    }

    return 0;
}

void CLINT::store64(uint64_t addr, uint64_t data)
{
    switch (addr) {
        case CLINT_MTIMECMP:
            mtimecmp = data;
            break;
        case CLINT_MTIME:
            mtime = data;
            break;
        default:
            break;
    }
}

std::pair<uint64_t, ReturnException> CLINT::load(uint64_t addr, size_t sz)
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

ReturnException CLINT::store(uint64_t addr, uint64_t value, size_t sz)
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
