/**
 * @file
 */

#ifndef DRIVER_BASE_HPP_
#define DRIVER_BASE_HPP_


namespace driver
{

class Base
{
public:
    static void init();

private:
    static void clock_init();
    static void enable_peripheral_clocks();
    static void configure_pins();
};

}  // namespace driver



#endif  // DRIVER_BASE_HPP_
