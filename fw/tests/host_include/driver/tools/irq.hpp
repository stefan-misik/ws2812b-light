#ifndef TEST_HOST_DRIVER_TOOLS_IRQ_HPP_
#define TEST_HOST_DRIVER_TOOLS_IRQ_HPP_

namespace driver
{
// Host transactions are single-threaded; there is no hardware interrupt.
class IrqGuard
{
public:
    IrqGuard() = default;
    ~IrqGuard() { }
};
}  // namespace driver

#endif