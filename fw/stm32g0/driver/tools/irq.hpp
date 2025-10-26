/**
 * @file
 */

#ifndef DRIVER_TOOLS_IRQ_HPP_
#define DRIVER_TOOLS_IRQ_HPP_

#include <cstdint>

#include "cmsis_compiler.h"


namespace driver
{

/**
 * @brief Tool used to disable Interrupts in RAII-style
 */
class IrqGuard
{
public:
    IrqGuard():
        primask_(disableAndGetPrimask())
    { }

    ~IrqGuard()
    {
        __DMB();
        __set_PRIMASK(primask_);
        __ISB();
    }

private:
    const std::uint32_t primask_;

    static std::uint32_t disableAndGetPrimask()
    {
        __DMB();
        const auto primask = __get_PRIMASK();
        __disable_irq();
        return primask;
    }
};



}  // namespace driver

#endif  // DRIVER_TOOLS_IRQ_HPP_

