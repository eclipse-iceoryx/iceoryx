# waitset - Callbacks

**Hint** This example shows a work in progress and is not yet working.

 WaitSet
  - subscriber, guards, publisher are attachable
  - in wait() arbitrary attach/detach allowed
      - returns list of triggered conditions
      - conditions has a possibility to identify itself as publisher,
      subscriber, guard
  - dtor: detaches all attached conditions
  - dtor conditions: detaches itself in the waitset
  - wait & callbacks

 ThreadsafeWaitSet
  - only callbacks?

## Introduction

