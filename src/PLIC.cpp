#include <PLIC.h>

PLIC::PLIC()
{
    pending = 0;
    senable = 0;
    spriority = 0;
    sclaim = 0;
}

std::pair<uint64_t, ReturnException> PLIC::load(uint64_t addr, size_t sz)
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

ReturnException PLIC::store(uint64_t addr, uint64_t value, size_t sz)
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

uint64_t PLIC::load64(uint64_t addr) const
{
    switch (addr) {
        case PLIC_PENDING:
            return pending;
        case PLIC_SENABLE:
            return senable;
        case PLIC_SPRIORITY:
            return spriority;
        case PLIC_SCLAIM:
            return sclaim;
        default:
            break;
    }

    return 0;
}

void PLIC::store64(uint64_t addr, uint64_t data)
{
    switch (addr) {
        case PLIC_PENDING:
            pending = data;
            break;
        case PLIC_SENABLE:
            senable = data;
            break;
        case PLIC_SPRIORITY:
            spriority = data;
            break;
        case PLIC_SCLAIM:
            sclaim = data;
            break;
        default:
            break;
    }
}
