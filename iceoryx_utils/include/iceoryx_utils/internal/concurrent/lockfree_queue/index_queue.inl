namespace iox
{
namespace concurrent
{
template <uint64_t Capacity, typename ValueType>
IndexQueue<Capacity, ValueType>::IndexQueue(ConstructEmpty_t) noexcept
    : m_readPosition(Index(Capacity))
    , m_writePosition(Index(Capacity))
{
    for (uint64_t i = 0u; i < Capacity; ++i)
    {
        m_cells[i].store(Index(0), std::memory_order_relaxed);
    }
}

template <uint64_t Capacity, typename ValueType>
IndexQueue<Capacity, ValueType>::IndexQueue(ConstructFull_t) noexcept
    : m_readPosition(Index(0u))
    , m_writePosition(Index(Capacity))
{
    for (uint64_t i = 0u; i < Capacity; ++i)
    {
        m_cells[i].store(Index(i), std::memory_order_relaxed);
    }
}

template <uint64_t Capacity, typename ValueType>
constexpr uint64_t IndexQueue<Capacity, ValueType>::capacity() const noexcept
{
    return Capacity;
}

template <uint64_t Capacity, typename ValueType>
void IndexQueue<Capacity, ValueType>::push(const ValueType index) noexcept
{
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

    constexpr bool NotPublished = true;

    auto writePosition = m_writePosition.load(std::memory_order_relaxed);
    do
    {
        auto oldValue = loadvalueAt(writePosition, std::memory_order_relaxed);

        auto cellIsFree = oldValue.isOneCycleBehind(writePosition);

        if (cellIsFree)
        {
            // case (1)
            Index newValue(index, writePosition.getCycle());

            // if publish fails, another thread has published before us
            bool published = m_cells[writePosition.getIndex()].compare_exchange_weak(
                oldValue, newValue, std::memory_order_relaxed, std::memory_order_relaxed);

            if (published)
            {
                break;
            }
        }

        // even if we are not able to publish, we check whether some other push has already updated the writePosition
        // before trying again to publish
        auto writePositionRequiresUpdate = oldValue.getCycle() == writePosition.getCycle();

        if (writePositionRequiresUpdate)
        {
            // case (2)
            // the writePosition was not updated yet by another push but the value was already written
            // help with the update
            // (note that we do not care if it fails, then a retry or another push will handle it)

            Index newWritePosition(writePosition + 1);
            m_writePosition.compare_exchange_strong(
                writePosition, newWritePosition, std::memory_order_relaxed, std::memory_order_relaxed);
        }
        else
        {
            // case (3) and (4)
            // note: we do not update with CAS here, the CAS is bound to fail anyway
            // (since our value of writePosition is not up to date so needs to be loaded again)
            writePosition = m_writePosition.load(std::memory_order_relaxed);
        }

    } while (NotPublished);

    Index newWritePosition(writePosition + 1);

    // if this compare-exchange fails it is no problem, this only delays the update of m_writePosition
    // for other pushes which are able to do them on their own (if writePositionRequiresUpdate above is true)
    // no one else except popIfFull requires this update:
    // In this case it is also ok: the push is only complete once this update of m_writePosition was executed,
    // and the queue (logically) cannot be full until this happens.
    m_writePosition.compare_exchange_strong(
        writePosition, newWritePosition, std::memory_order_relaxed, std::memory_order_relaxed);
}

template <uint64_t Capacity, typename ValueType>
bool IndexQueue<Capacity, ValueType>::pop(ValueType& index) noexcept
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

    bool ownershipGained = false;
    Index value;
    auto readPosition = m_readPosition.load(std::memory_order_relaxed);
    do
    {
        value = loadvalueAt(readPosition, std::memory_order_relaxed);

        // we only dequeue if value and readPosition are in the same cycle
        auto cellIsValidToRead = readPosition.getCycle() == value.getCycle();

        if (cellIsValidToRead)
        {
            // case (1)
            Index newReadPosition(readPosition + 1);
            ownershipGained = m_readPosition.compare_exchange_weak(
                readPosition, newReadPosition, std::memory_order_relaxed, std::memory_order_relaxed);
        }
        else
        {
            // readPosition is ahead by one cycle, queue was empty at value load
            auto isEmpty = value.isOneCycleBehind(readPosition);

            if (isEmpty)
            {
                // case (2)
                return false;
            }

            // case (3) and (4) requires loading readPosition again
            readPosition = m_readPosition.load(std::memory_order_relaxed);
        }

        // readPosition is outdated, retry operation

    } while (!ownershipGained); // we leave if we gain ownership of readPosition

    index = value.getIndex();
    return true;
}

template <uint64_t Capacity, typename ValueType>
bool IndexQueue<Capacity, ValueType>::popIfFull(ValueType& index) noexcept
{
    // we do NOT need a CAS loop here since if we detect that the queue is not full
    // someone else popped an element and we do not retry to check whether it was filled AGAIN
    // concurrently (which will usually not be the case and then we would return false anyway)
    // if it is filled again we can (and will) retry popIfFull from the call site

    // the queue is full if and only if write position and read position are the same but read position is
    // one cycle behind write position
    // unfortunately it seems impossible in this design to check this condition without loading
    // write posiion and read position (which causes more contention)

    auto writePosition = m_writePosition.load(std::memory_order_relaxed);
    auto readPosition = m_readPosition.load(std::memory_order_relaxed);
    auto value = loadvalueAt(readPosition, std::memory_order_relaxed);

    auto isFull = writePosition.getIndex() == readPosition.getIndex() && readPosition.isOneCycleBehind(writePosition);

    if (isFull)
    {
        Index newReadPosition(readPosition + 1);
        auto ownershipGained = m_readPosition.compare_exchange_strong(
            readPosition, newReadPosition, std::memory_order_relaxed, std::memory_order_relaxed);

        if (ownershipGained)
        {
            index = value.getIndex();
            return true;
        }
    }

    // otherwise someone else has dequeued an index and the queue was not full at the start of this popIfFull
    return false;
}

template <uint64_t Capacity, typename ValueType>
bool IndexQueue<Capacity, ValueType>::empty() const noexcept
{
    auto readPosition = m_readPosition.load(std::memory_order_relaxed);
    auto value = loadvalueAt(readPosition, std::memory_order_relaxed);

    // if m_readPosition is ahead by one cycle compared to the value stored at head,
    // the queue was empty at the time of the loads above (but might not be anymore!)
    return value.isOneCycleBehind(readPosition);
}

template <uint64_t Capacity, typename ValueType>
void IndexQueue<Capacity, ValueType>::push(UniqueIndex& index) noexcept
{
    push(index.release());
}

template <uint64_t Capacity, typename ValueType>
typename IndexQueue<Capacity, ValueType>::UniqueIndex IndexQueue<Capacity, ValueType>::pop() noexcept
{
    ValueType value;
    if (pop(value))
    {
        return UniqueIndex(value);
    }
    return UniqueIndex::invalid;
}

template <uint64_t Capacity, typename ValueType>
typename IndexQueue<Capacity, ValueType>::UniqueIndex IndexQueue<Capacity, ValueType>::popIfFull() noexcept
{
    ValueType value;
    if (popIfFull(value))
    {
        return UniqueIndex(value);
    }
    return UniqueIndex::invalid;
}

template <uint64_t Capacity, typename ValueType>
typename IndexQueue<Capacity, ValueType>::Index
IndexQueue<Capacity, ValueType>::loadvalueAt(const Index& position, std::memory_order memoryOrder) const
{
    return m_cells[position.getIndex()].load(memoryOrder);
}


} // namespace concurrent
} // namespace iox
