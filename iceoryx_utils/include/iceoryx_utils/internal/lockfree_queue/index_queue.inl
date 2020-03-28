namespace iox
{
template <uint64_t Capacity>
IndexQueue<Capacity>::IndexQueue(ConstructEmpty)
    : m_head(Index(Capacity))
    , m_tail(Index(Capacity))
{
}

template <uint64_t Capacity>
IndexQueue<Capacity>::IndexQueue(ConstructFull)
    : m_head(Index(0))
    , m_tail(Index(Capacity))
{
    for (uint64_t i = 0; i < Capacity; ++i)
    {
        m_values[i].store(Index(i));
    }
}

template <uint64_t Capacity>
constexpr uint64_t IndexQueue<Capacity>::capacity()
{
    return Capacity;
}

template <uint64_t Capacity>
void IndexQueue<Capacity>::push(indexvalue_t index)
{
    Index oldTail;
    do
    {
        oldTail = m_tail.load(std::memory_order_acquire);

        auto position = oldTail.getIndex();

        auto value = m_values[position].load(std::memory_order_relaxed);
        auto tailCycle = oldTail.getCycle();
        auto valueCycle = value.getCycle();

        if (valueCycle == tailCycle)
        {
            // try to help moving the m_tail, can fail but we retry anyway and some enqueuer will succeed
            m_tail.compare_exchange_strong(oldTail, oldTail + 1, std::memory_order_acq_rel, std::memory_order_acquire);
            continue; // retry operation
        }

        if (valueCycle + 1 != tailCycle)
        {
            // valueCycle is different and m_tail not ahead by one cycle, hence oldTail is stale
            continue; // retry operation
        }

        // we only insert a new value if m_tail is ahead by one cycle

        // technically this can overwrite a value that was not yet dequeued, but in our use case
        // this will not happen since we transfer indices back and forth from 2 identically
        // sized queues containing only capacity (= length of a cycle) m_values in total

        Index newValue(index, tailCycle);

        if (m_values[position].compare_exchange_strong(
                value, newValue, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            break; // push successful
        }

    } while (true); // we leave iff the CAS was successful

    // try moving the m_tail, if it fails it is no problem, another push has to help this operation
    // before it can succeed itself
    m_tail.compare_exchange_strong(oldTail, oldTail + 1, std::memory_order_relaxed, std::memory_order_relaxed);
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::pop(indexvalue_t& index)
{
    Index value;
    do
    {
        auto oldHead = m_head.load(std::memory_order_acquire);
        value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed); // (value load)
        auto headCycle = oldHead.getCycle();
        auto valueCycle = value.getCycle();

        if (valueCycle != headCycle)
        {
            if (valueCycle + 1 == headCycle)
            {
                return false; // m_head is ahead by one cycle, queue was empty at (value load)
            }
            continue; // oldHead is stale, retry operation
        }

        // we only dequeue if the cycle of value and of m_head is the same

        if (m_head.compare_exchange_strong(oldHead, oldHead + 1, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            break; // pop successful
        }

    } while (true); // we leave iff the CAS was successful

    index = value.getIndex();
    return true;
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::popIfFull(indexvalue_t& index)
{
    Index value;
    do
    {
        auto oldHead = m_head.load(std::memory_order_acquire);
        value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed); // (value load)
        auto headCycle = oldHead.getCycle();
        auto valueCycle = value.getCycle();

        if (valueCycle != headCycle)
        {
            if (valueCycle + 1 == headCycle)
            {
                return false; // m_head is ahead by one cycle, queue was empty at (value load)
            }
            continue; // oldHead is stale, retry operation
        }

        // check whether the queue is full (can we do this without tail as with empty?)
        auto oldTail = m_tail.load(std::memory_order_relaxed);
        if (oldTail.getIndex() == oldHead.getIndex() && oldTail.getCycle() == headCycle + 1)
        {
            // queue is full, this can only change when m_head changes which will be detected by CAS

            // we only dequeue if the cycle of value and of m_head is the same
            if (m_head.compare_exchange_strong(
                    oldHead, oldHead + 1, std::memory_order_acq_rel, std::memory_order_acquire))
            {
                break; // pop successful
            }
        }
        else
        {
            return false; // queue is not full
        }

    } while (true); // we leave iff the CAS was successful

    index = value.getIndex();
    return true;
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::empty()
{
    auto oldHead = m_head.load(std::memory_order_acquire);
    auto value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed);

    if (value.getCycle() + 1 == oldHead.getCycle())
    {
        // if m_head is ahead by one cycle compared to the value stored at head,
        // the queue was empty at the time of the loads ebove (but might not be anymore!)
        return true;
    }
    return false;
}
} // namespace iox