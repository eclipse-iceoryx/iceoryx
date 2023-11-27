# iceoryx v2.0.3

## [v2.0.3](https://github.com/eclipse-iceoryx/iceoryx/tree/v2.0.3) (2023-02-02)

[Full Changelog](https://github.com/eclipse-iceoryx/iceoryx/compare/v2.0.2...v2.0.3)

**Bugfixes:**

- Build error on certain versions of Windows/Visual Studio [\#1476](https://github.com/eclipse-iceoryx/iceoryx/issues/1476)
- Fix FileLock leak when lock failed [\#1440](https://github.com/eclipse-iceoryx/iceoryx/issues/1440)
- Fix INTERFACE_INCLUDE_DIRECTORIES in CMake [\#1481](https://github.com/eclipse-iceoryx/iceoryx/issues/1481)
- `m_originId` in `mepoo::ChunkHeader` sometimes not set [\#1668](https://github.com/eclipse-iceoryx/iceoryx/issues/1668)
- `WaitSet::wait` returns if data was send before `WaitSet::attachState(.., State::HAS_{DATA, REQUEST, RESPONSE})` [\#1855](https://github.com/eclipse-iceoryx/iceoryx/issues/1855)
