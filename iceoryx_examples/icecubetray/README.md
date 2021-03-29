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

```
                                 +-----------------------+
+--------------------+           |                       |           +---------------------+
|   Radar App        |           |  Privileged Shared    |           |   Some Other App    |
|   @perception      |           |  Memory Segment       |           |   @perception       |
|                    |  publish  |                       | subscribe |                     |
|                    |  -------> |  r: unprivileged      |  ------>  |                     |
|                    |           |  w: privileged        |           |                     |
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
                                 |  Infotainment Shared  |           |   Display App       |
                                 |  Memory Segment       |  publish  |   @infotainment     |
                                 |                       |  <------  |                     |
                                 |  r: infotainment      |           |                     |
                                 |  w: infotainment      |           |                     |
                                 |                       |           |                     |
                                 |                       |           |                     |
                                 |                       |           |                     |
                                 |                       |           +---------------------+
                                 +-----------------------+
```

It's advised to create per writer group only one shared memory segement (e.g. not two segements with `w: infotainment`).
In this case it wouldn't be possible to control which segment will be used.

!!! note
    Note the shared memory managment segment is always available for everyone to **read** and **write**

### Radar App

### Display App
