/**
 * @file
 */

#ifndef DRIVER_USART_TX_HPP_
#define DRIVER_USART_TX_HPP_

#include <memory>
#include <utility>

#include "tools/hidden.hpp"
#include "tools/bip_buffer_controller.hpp"
#include "driver/common.hpp"


namespace driver
{

/**
 * @brief DMA-driven transmit-only USART driver
 */
class UsartTx
{
public:
    static const inline std::size_t BUFFER_SIZE = 256;
    static const inline std::uint32_t DEFAULT_BAUD_RATE = 115200;

    UsartTx();
    ~UsartTx();

    /**
     * @brief Configure USART and its TX DMA channel
     *
     * @param usart_id USART identifier
     * @param dma_channel_id DMA channel identifier
     * @param baud_rate USART baud rate
     * @param buffer_size Number of bytes to allocate for the transmit buffer
     *
     * @return Success
     */
    bool initialize(UsartId usart_id, DmaChannelId dma_channel_id, std::uint32_t baud_rate = DEFAULT_BAUD_RATE,
        std::size_t buffer_size = BUFFER_SIZE);

    /**
     * @brief Reserve a contiguous region of the buffer for writing
     *
     * @param size Preferred number of bytes to reserve
     *
     * @return Pair of the region pointer and its reserved length
     *
     * @note The method is non-const only to allow obtaining a non-const pointer
     *       to the buffer. The method does not mutate of the object and is
     *       therefore safe to call repeatedly and/or throw away the result with
     *       no further action.
     */
    std::pair<void *, std::size_t> reserveBuffer(std::size_t size = BUFFER_SIZE);

    /**
     * @brief Commit data to the buffer for transmission
     *
     * @param size Number of bytes to commit
     * @param buffer Pointer to the buffer obtained from @ref reserveBuffer()
     *
     * @return Success
     */
    bool commitBuffer(void * buffer, std::size_t size);

    /**
     * @brief Handle the DMA1 channel 4 interrupt
     */
    void maybeHandleDmaInterrupt();

private:
    using SizeType = std::uint16_t;

    struct Private;

    Hidden<Private, sizeof(void *) + sizeof(std::uint32_t)> p_;

    BipBufferController<SizeType> buffer_ctrl_{0};
    SizeType dma_active_size_ = 0;
    SizeType buffer_size_ = 0;

    std::unique_ptr<std::uint8_t[]> buffer_;
};

}  // namespace driver

#endif  // DRIVER_USART_TX_HPP_