// Copyright 2021 GHA Test Team
#include "TimedDoor.h"

#include <chrono>
#include <stdexcept>
#include <thread>

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
  if (door.isDoorOpened()) {
    door.throwState();
  }
}

TimedDoor::TimedDoor(int timeout) : adapter(new DoorTimerAdapter(*this)),
                                   iTimeout(timeout),
                                   isOpened(false) {}

bool TimedDoor::isDoorOpened() {
  return isOpened;
}

void TimedDoor::unlock() {
  isOpened = true;
  Timer timer;
  timer.tregister(iTimeout, adapter);
}

void TimedDoor::lock() {
  isOpened = false;
}

int TimedDoor::getTimeOut() const {
  return iTimeout;
}

void TimedDoor::throwState() {
  throw std::runtime_error("Door is opened too long");
}

void Timer::sleep(int timeoutMs) {
  if (timeoutMs <= 0) {
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
}

void Timer::tregister(int timeoutMs, TimerClient* c) {
  client = c;
  sleep(timeoutMs);
  if (client) {
    client->Timeout();
  }
}
