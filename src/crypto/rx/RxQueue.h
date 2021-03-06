/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2019 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      Lee Clagett <https://github.com/vtnerd>
 * Copyright 2018-2019 tevador     <tevador@gmail.com>
 * Copyright 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2020 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XMRIG_RX_QUEUE_H
#define XMRIG_RX_QUEUE_H


#include "base/io/Async.h"
#include "base/tools/Object.h"
#include "crypto/common/HugePagesInfo.h"
#include "crypto/rx/RxConfig.h"
#include "crypto/rx/RxSeed.h"


#include <condition_variable>
#include <mutex>
#include <thread>


using uv_async_t = struct uv_async_s;


namespace xmrig
{


class IRxListener;
class IRxStorage;
class RxDataset;


class RxQueueItem
{
public:
    RxQueueItem(const RxSeed &seed, const std::vector<uint32_t> &nodeset, uint32_t threads, bool hugePages, bool oneGbPages, RxConfig::Mode mode, int priority) :
        hugePages(hugePages),
        oneGbPages(oneGbPages),
        priority(priority),
        mode(mode),
        seed(seed),
        nodeset(nodeset),
        threads(threads)
    {}

    const bool hugePages;
    const bool oneGbPages;
    const int priority;
    const RxConfig::Mode mode;
    const RxSeed seed;
    const std::vector<uint32_t> nodeset;
    const uint32_t threads;
};


class RxQueue
{
public:
    XMRIG_DISABLE_COPY_MOVE(RxQueue);

    RxQueue(IRxListener *listener);
    ~RxQueue();

    HugePagesInfo hugePages();
    RxDataset *dataset(const Job &job, uint32_t nodeId);
    template<typename T> bool isReady(const T &seed);
    void enqueue(const RxSeed &seed, const std::vector<uint32_t> &nodeset, uint32_t threads, bool hugePages, bool oneGbPages, RxConfig::Mode mode, int priority);

private:
    enum State {
        STATE_IDLE,
        STATE_PENDING,
        STATE_SHUTDOWN
    };

    template<typename T> bool isReadyUnsafe(const T &seed) const;
    void backgroundInit();
    void onReady();

    IRxListener *m_listener = nullptr;
    IRxStorage *m_storage   = nullptr;
    RxSeed m_seed;
    State m_state = STATE_IDLE;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::thread m_thread;
    std::vector<RxQueueItem> m_queue;
    uv_async_t *m_async     = nullptr;
};


} /* namespace xmrig */


#endif /* XMRIG_RX_QUEUE_H */
