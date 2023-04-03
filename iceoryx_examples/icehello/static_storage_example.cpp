#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/string.hpp"

#include <iostream>

class StaticStorage
{
    static constexpr const char STATIC_STORAGE_FILE_SUFFIX[] = ".toml";
    static constexpr uint64_t STATIC_STORAGE_FILE_SUFFIX_LENGTH = sizeof(STATIC_STORAGE_FILE_SUFFIX) / sizeof(char);
    static constexpr uint64_t FILENAME_LENGTH = 128 - STATIC_STORAGE_FILE_SUFFIX_LENGTH;
    using FileName_t = iox::string<FILENAME_LENGTH>;

    StaticStorage(FileName_t name)
        : m_name{name}
    {
    }
    friend class StaticStorageBuilder;
    FileName_t m_name = "";

  public:
    void print_name() noexcept
    {
        std::cout << m_name << std::endl;
    }
};

enum class StaticStorageError
{
    FILE_CREATION_FAILED,
    PERMISSION_ERROR,
};

class StaticStorageBuilder
{
    IOX_BUILDER_PARAMETER(StaticStorage::FileName_t, name, "")
  public:
    iox::expected<StaticStorage, StaticStorageError> create() noexcept
    {
        // Create file with posix_user and group rights
        // Write to the file
        // Make the file read access only
        return iox::success<StaticStorage>(StaticStorage{m_name});
    }
};

int main()
{
    // auto static_storage = StaticStorage{"not_allowed"};
    auto static_storage = StaticStorageBuilder().name("test").create();

    static_storage.and_then([](auto& static_storage) { static_storage.print_name(); });
}
