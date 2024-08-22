/**
 * @file
 */

#ifndef DRIVER_LED_CONTROLLER_HPP_
#define DRIVER_LED_CONTROLLER_HPP_

#include <cstddef>
#include <cstdint>


namespace driver
{

class LedController
{
public:
    enum class TimerId
    {
        TIM_1,
    };

    static bool initializeTimer(TimerId tim_id);
    static bool startTimer(TimerId tim_id);

    LedController();
    ~LedController();

    /**
     * @brief Initialize  current instance for specified timer and channel ID
     *
     * @param tim_id Timer whose channel to initialize (it already needs to be initialized)
     * @param channel_id Channel to associate the Led Controller instance with
     * @return Success
     */
    bool initialize(TimerId tim_id, uint8_t channel_id);

    void forcePwm(std::uint16_t value);

private:
    struct Private;
    struct PrivateStorage
    {
        char data[8] alignas(std::uintptr_t);
    };

    Private & p() { return *reinterpret_cast<Private *>(&p_); }
    const Private & p() const { return *reinterpret_cast<const Private *>(&p_); }

    PrivateStorage p_;
};

}  // driver


#endif  // DRIVER_LED_CONTROLLER_HPP_