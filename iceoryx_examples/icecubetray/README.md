# icecubetray

## Introduction

This example demonstrates how access rights can be applied to shared memory segments.
It provides a custom RouDi, a radar and a display application.

## Expected output

<!-- Add asciinema link here -->

## Code walkthrough



| Users        | privileged group | unprivileged group | infotainment group |   iceoryx group    |
|--------------|:----------------:|:------------------:|:------------------:|:------------------:|
| perception   |        X         |                    |                    |         X          |
| infotainment |                  |         X          |         X          |         X          |
| roudi        |                  |                    |                    |         X          |

### RouDi with static shared memory segments

The segments are configured as depicted below:

#### Positive Scenario
```
                                 +-----------------------+
+--------------------+           |                       |           +---------------------+
|   Radar App        |           | Privileged Shared     |           |  Preprocessing App  |
|   @perception      |           | Memory Segment        |           |  @perception        |
|                    |  publish  |                       | subscribe |                     |
|                    |  -------> | r group: unprivileged |  ------>  |                     |
|                    |           | w group: privileged   |           |                     |
|                    |           |                       |           |                     |
|                    |           |                       |  <------  |                     |
|                    |           |                       |  publish  |                     |
+--------------------+           |                       |           +---------------------+
                                 +-----------------------+
                                                 |
                                                 |          subscribe
                                                 +---------------------------+
                                                                             |
                                                                             |
                                                                             |
                                                                            \ /
                                 +-----------------------+
                                 |                       |           +---------------------+
                                 | Infotainment Shared   |           |   Display App       |
                                 | Memory Segment        |  publish  |   @infotainment     |
                                 |                       |  <------  |                     |
                                 | r group: infotainment |           |                     |
                                 | w group: infotainment |           |                     |
                                 |                       |           |                     |
                                 |                       |           |                     |
                                 |                       |           |                     |
                                 |                       |           +---------------------+
                                 +-----------------------+
```

It's advised to create per writer group only one shared memory segement (e.g. not two segements with `w: infotainment`).
In this case it wouldn't be possible to control which segment will be used. (=> add a test for that)

!!! note
    Note the shared memory managment segment is always available for everyone to **read** and **write**

#### Negative Scenario

Start preprocessing app with 'dummy' user, should not get not_null segfault in RouDi, but something like "Access denied, no shared memory payload segment available"



### Radar App

### Display App
