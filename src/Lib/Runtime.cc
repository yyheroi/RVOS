/**
 * @file Runtime.cc
 * @author Swan BaiLei27 (https://github.com/BaiLei27)
 * @brief
 * @version 0.1
 * @date 2026/04/13/21:04
 *
 * @copyright Copyright (c) 2026 Swan BaiLei27 <SwanBaiLei2Seven@foxmail.com>
 *
 */

#include <cstddef>
extern "C" {
    void *malloc(std::size_t size);
    void free(void *pMem);
}

void *operator new (std::size_t size)
{
    return malloc(size);
}

void operator delete (void *pMem) noexcept
{
    free(pMem);
}

void operator delete (void *pMem, std::size_t /*unused*/) noexcept
{
    operator delete (pMem);
}
