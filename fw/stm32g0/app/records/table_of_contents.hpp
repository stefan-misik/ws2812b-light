/**
 * @file
 */

#ifndef APP_RECORDS_TABLE_OF_CONTENTS_HPP_
#define APP_RECORDS_TABLE_OF_CONTENTS_HPP_

#include <cstddef>
#include <cstdint>
#include <array>
#include <algorithm>


/**
 * @brief Entry in the table of contents
 */
struct TableOfContentsEntry
{
    using PageId = std::uint16_t;
    static constexpr PageId INVALID_PAGE_ID = 0x8000u;

    std::uint16_t page_id = INVALID_PAGE_ID;
    std::uint16_t epoch = 0u;

    static bool isNewer(std::uint16_t epoch, std::uint16_t previous)
    {
        // Lets assume that the higher epoch in some reasonable range is the newer
        // one, not wrapped around stale epochs.
        const auto difference = static_cast<std::uint16_t>(epoch - previous);
        return difference < 0x8000u;
    }
};


/**
 * @brief Class managing the table of contents for the records
 */
template<std::size_t SZ>
class TableOfContents
{
public:
    using TocEntry = TableOfContentsEntry;
    using PageId = TocEntry::PageId;

    static constexpr std::size_t MAX_RECORDS = SZ;
    static constexpr PageId INVALID_PAGE_ID = TocEntry::INVALID_PAGE_ID;

    TocEntry & operator[](std::size_t index) { return toc_[index]; }
    const TocEntry & operator[](std::size_t index) const { return toc_[index]; }

    void reset()
    {
        for (auto & entry : toc_)
            entry.page_id = INVALID_PAGE_ID;
    }

    bool isPageTaken(PageId page_id) const
    {
        return std::any_of(toc_.begin(), toc_.end(),
            [page_id](const TocEntry & entry)
            {
                return entry.page_id == page_id;
            });
    }

private:
    std::array<TocEntry, MAX_RECORDS> toc_;
};

#endif  // APP_RECORDS_TABLE_OF_CONTENTS_HPP_
