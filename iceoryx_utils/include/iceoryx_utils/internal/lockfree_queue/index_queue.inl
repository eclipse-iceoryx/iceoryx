namespace iox
{
template <uint64_t Capacity>
IndexQueue<Capacity>::IndexQueue(ConstructEmpty_t)
    : m_readIndex(Index(Capacity))
    , m_writeIndex(Index(Capacity))
{
}

template <uint64_t Capacity>
IndexQueue<Capacity>::IndexQueue(ConstructFull_t)
    : m_readIndex(Index(0))
    , m_writeIndex(Index(Capacity))
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
void IndexQueue<Capacity>::push(value_t uniqueIndex)
{
    constexpr bool notPublished = true;

    // we need the CAS loop here since we may fail due to concurrent push operations
    // note that we are always able to succeed to publish since we have
    // enough capacity for all unique indices used

    // case analyis
    // (1) loaded value is exactly one cycle behind:
    //     value is from the last cycle
    //     we can try to publish
    // (2) loaded value has the same cycle:
    //     some other push has published but not updated the write position
    //     help updating the write position
    // (3) loaded value is more than one cycle behind:
    //     this should only happen due to wrap around when push is interrupted for a long time
    //     reload write position and try again
    //     note that a complete wraparound can lead to a false detection of 1) (ABA problem)
    //     but this is very unlikely with e.g. a 64 bit value type
    // (4) loaded value is some cycle ahead:
    //     write position is outdated, there must have been other pushes concurrently
    //     reload write position and try again

    auto writePosition = loadNextWritePosition();
    do
    {
        auto oldValue = loadValueAt(writePosition);


        auto isFreeToPublish = [&]() { return oldValue.isOneCycleBehind(writePosition); };

        if (isFreeToPublish())
        {
            // case (1)
            Index newValue(uniqueIndex, writePosition.getCycle());
            // if publish fails, another thread has published before
            auto published = tryToPublishAt(writePosition, oldValue, newValue);
            if (published)
            {
                break;
            }
        }

        // even if we are not able to publish, we try to help moving the writePosition
        // before trying again to publish
        auto helpUpdateWritePosition = [&]() { return oldValue.getCycle() == writePosition.getCycle(); };

        if (helpUpdateWritePosition())
        {
            // case (2)
            updateNextWritePosition(writePosition);
        }
        else
        {
            // case (3) and (4)
            // note: we do not the CAS in updateNextWritePosition here, it is bound to fail anyway
            writePosition = loadNextWritePosition();
        }

    } while (notPublished);

    updateNextWritePosition(writePosition);
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::pop(value_t& uniqueIndex)
{
    // we need the CAS loop here since we may fail due to concurrent pop operations
    // we leave when we detect an empty queue, otherwise we retry the pop operation

    // case analyis
    // (1) loaded value has the same cycle:
    //     value was not popped before
    //     try to get ownership
    // (2) loaded value is exactly one cycle behind:
    //     value is from the last cycle which means the queue is empty
    //     return false
    // (3) loaded value is more than one cycle behind:
    //     this should only happen due to wrap around when push is interrupted for a long time
    //     reload read position and try again
    // (4) loaded value is some cycle ahead:
    //     read position is outdated, there must have been pushes concurrently
    //     reload read position and try again

    Index value;
    do
    {
        auto readPosition = loadNextReadPosition();
        value = loadValueAt(readPosition);

        // we only dequeue if value and readPosition are in the same cycle
        auto isUpToDate = [&]() { return readPosition.getCycle() == value.getCycle(); };
        // readPosition is ahead by one cycle, queue was empty at loadValueAt(..)
        auto isEmpty = [&]() { return value.isOneCycleBehind(readPosition); };

        if (isUpToDate())
        {
            // case (1)
            if (tryToGetOwnership(readPosition))
            {
                break; // pop successful
            }
        }
        else if (isEmpty())
        {
            // case (2)
            return false;
        }
        // else readPosition is outdated, retry operation

        // case (3) and (4)

    } while (true); // we leave if we get ownership of readPosition

    uniqueIndex = value.getIndex();
    return true;
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::popIfFull(value_t& uniqueIndex)
{
    // we do NOT need a CAS loop here since if we detect that the queue is not full
    // someone else popped an element and we do not retry to check whether it was filled AGAIN
    // concurrently (which will usually not be the case and then we would return false anyway)
    // if it is filled again we can (and will) retry popIfFull from the call site

    // the queue is full if and only if write position and read position are the same but read position is
    // one cycle behind write position
    // unfortunately it seems impossible in this design to check this condition without loading
    // write posiion and read position (which causes more contention)

    auto writePosition = loadNextWritePosition();
    auto readPosition = loadNextReadPosition();
    auto value = loadValueAt(readPosition);

    auto isFull = [&]() {
        return writePosition.getIndex() == readPosition.getIndex() && readPosition.isOneCycleBehind(writePosition);
    };

    if (isFull())
    {
        if (tryToGetOwnership(readPosition))
        {
            uniqueIndex = value.getIndex();
            return true;
        }
    } // else someone else has popped an index
    return false;
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::empty()
{
    auto oldReadIndex = m_readIndex.load(std::memory_order_acquire);
    auto value = m_values[oldReadIndex.getIndex()].load(std::memory_order_relaxed);

    // if m_readIndex is ahead by one cycle compared to the value stored at head,
    // the queue was empty at the time of the loads above (but might not be anymore!)
    return value.isOneCycleBehind(oldReadIndex);
}

template <uint64_t Capacity>
typename IndexQueue<Capacity>::Index IndexQueue<Capacity>::loadNextReadPosition() const
{
    // return m_readIndex.load(std::memory_order_relaxed);
    return m_readIndex.load(std::memory_order_acquire);
}

template <uint64_t Capacity>
typename IndexQueue<Capacity>::Index IndexQueue<Capacity>::loadNextWritePosition() const
{
    // return m_writeIndex.load(std::memory_order_relaxed);
    return m_writeIndex.load(std::memory_order_acquire);
}

template <uint64_t Capacity>
typename IndexQueue<Capacity>::Index
IndexQueue<Capacity>::loadValueAt(typename IndexQueue<Capacity>::Index position) const
{
    return m_values[position.getIndex()].load(std::memory_order_relaxed);
    // return m_values[position.getIndex()].load(std::memory_order_acquire);
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::tryToPublishAt(typename IndexQueue<Capacity>::Index writePosition,
                                          Index& oldValue,
                                          Index newValue)
{
    return m_values[writePosition.getIndex()].compare_exchange_strong(
        oldValue, newValue, std::memory_order_acq_rel, std::memory_order_acquire);
}

template <uint64_t Capacity>
void IndexQueue<Capacity>::updateNextWritePosition(Index& writePosition)
{
    // compare_exchange updates oldWritePosition
    // if not updateSucceeded
    // else oldWritePosition stays unchanged
    // and will be updated in if(updateSucceeded)
    Index newWritePosition(writePosition + 1);
    auto updateSucceeded = m_writeIndex.compare_exchange_strong(
        writePosition, newWritePosition, std::memory_order_acq_rel, std::memory_order_acquire);

    // not needed
    // if (updateSucceeded)
    // {
    //     oldWritePosition = newWritePosition;
    // }
}

template <uint64_t Capacity>
bool IndexQueue<Capacity>::tryToGetOwnership(Index& oldReadPosition)
{
    Index newReadPosition(oldReadPosition + 1);
    return m_readIndex.compare_exchange_strong(
        oldReadPosition, newReadPosition, std::memory_order_acq_rel, std::memory_order_acquire);
}


template <uint64_t Capacity>
void IndexQueue<Capacity>::push(const UniqueIndex& index)
{
    push(*index);
}

template <uint64_t Capacity>
typename IndexQueue<Capacity>::UniqueIndex IndexQueue<Capacity>::pop()
{
    value_t value;
    if (pop(value))
    {
        return UniqueIndex(value);
    }
    return UniqueIndex::invalid;
}

template <uint64_t Capacity>
typename IndexQueue<Capacity>::UniqueIndex IndexQueue<Capacity>::popIfFull()
{
    value_t value;
    if (popIfFull(value))
    {
        return UniqueIndex(value);
    }
    return UniqueIndex::invalid;
}


} // namespace iox