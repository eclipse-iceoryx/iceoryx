#ifndef IOX_HOOFS_POSIX_WRAPPER_THREAD_INL
#define IOX_HOOFS_POSIX_WRAPPER_THREAD_INL

#include "iceoryx_hoofs/posix_wrapper/thread.hpp"

namespace iox
{
namespace posix
{
template <typename Signature, typename... CallableArgs>
inline cxx::expected<ThreadError> ThreadBuilder::create(cxx::optional<thread>& uninitializedThread,
                                                        const cxx::function<Signature>& callable,
                                                        CallableArgs&&... args) noexcept
{
    if (!callable)
    {
        std::cerr << "The thread cannot be created with an empty callable." << std::endl;
        return cxx::error<ThreadError>(ThreadError::EMPTY_CALLABLE);
    }

    uninitializedThread.emplace();
    uninitializedThread->m_callable = [=] { callable(std::forward<CallableArgs>(args)...); };

    // set attributes
    pthread_attr_t attr;
    auto initResult = posixCall(pthread_attr_init)(&attr).successReturnValue(0).evaluate();
    if (initResult.has_error())
    {
        uninitializedThread->m_isJoinable = false;
        uninitializedThread.reset();
        return cxx::error<ThreadError>(thread::errnoToEnum(initResult.get_error().errnum));
    }

    auto setDetachStateResult =
        posixCall(pthread_attr_setdetachstate)(&attr, m_detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE)
            .successReturnValue(0)
            .evaluate();
    if (setDetachStateResult.has_error())
    {
        uninitializedThread->m_isJoinable = false;
        uninitializedThread.reset();
        std::cerr << "Something went wrong when setting the detach state. This should never happen!" << std::endl;
        return cxx::error<ThreadError>(thread::errnoToEnum(setDetachStateResult.get_error().errnum));
    }
    uninitializedThread->m_isJoinable = !m_detached;

    // create thread
    auto createResult = posixCall(pthread_create)(
                            &uninitializedThread->m_threadHandle, &attr, thread::cbk, &uninitializedThread->m_callable)
                            .successReturnValue(0)
                            .evaluate();
    posixCall(pthread_attr_destroy)(&attr).successReturnValue(0).evaluate().or_else([](auto&) {
        std::cerr << "Something went wrong when destroying the thread attributes object." << std::endl;
    }); /// @todo not clear if pthread_attr_destroy can fail, specifications differ. Do we have to care if it fails?
    if (createResult.has_error())
    {
        uninitializedThread->m_isJoinable = false;
        uninitializedThread.reset();
        return cxx::error<ThreadError>(thread::errnoToEnum(createResult.get_error().errnum));
    }

    return cxx::success<>();
}
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_THREAD_INL
