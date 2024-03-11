= How to Run this example

1. Edit `IOX_MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY` in `iceoryx_posh/BUILD.bazel` to the desired size. This value is the `n` in the `n^2` time complexity this example is demonstrating.

2. Run Roudi with the `roudi_config.toml` in this folder. 

3. Run the publisher.

4. Run the subscriber.

You should observe for `n=100000` that it keeps running well past a few minutes.
