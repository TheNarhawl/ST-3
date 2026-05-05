// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <chrono>
#include <thread>
#include "TimedDoor.h"

using ::testing::StrictMock;

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class TimedDoorFixture : public ::testing::Test {
 protected:
  TimedDoorFixture() : door(20) {}

  void SetUp() override {
    door.lock();
  }

  void TearDown() override {}

  TimedDoor door;
};

TEST_F(TimedDoorFixture, CtorStoresTimeout) {
  EXPECT_EQ(door.getTimeOut(), 20);
}

TEST_F(TimedDoorFixture, LockClosesDoor) {
  door.lock();
  EXPECT_FALSE(door.isDoorOpened());
}

TEST(TimedDoor, UnlockWithZeroTimeoutThrows) {
  TimedDoor d(0);
  d.lock();
  EXPECT_THROW(d.unlock(), std::runtime_error);
}

TEST(TimedDoor, UnlockDoesNotThrowIfLockedBeforeTimeout) {
  TimedDoor d(30);
  d.lock();

  std::thread closer([&d]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    d.lock();
  });

  EXPECT_NO_THROW(d.unlock());
  closer.join();
}

TEST(TimedDoor, UnlockThrowsIfStillOpenedAtTimeout) {
  TimedDoor d(5);
  d.lock();
  EXPECT_THROW(d.unlock(), std::runtime_error);
}

TEST(Timer, CallsTimeoutOnRegisteredClient) {
  Timer timer;
  StrictMock<MockTimerClient> client;
  EXPECT_CALL(client, Timeout()).Times(1);
  timer.tregister(0, &client);
}

TEST(Timer, SleepsAtLeastRequestedDuration) {
  Timer timer;
  StrictMock<MockTimerClient> client;
  EXPECT_CALL(client, Timeout()).Times(1);

  const auto start = std::chrono::steady_clock::now();
  timer.tregister(10, &client);
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);

  EXPECT_GE(elapsed.count(), 10);
}

TEST(DoorTimerAdapter, TimeoutThrowsWhenDoorOpened) {
  TimedDoor d(50);
  d.lock();
  DoorTimerAdapter adapter(d);

  std::thread unlocker([&d]() {
    EXPECT_NO_THROW(d.unlock());
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);

  d.lock();
  unlocker.join();
}

TEST(DoorTimerAdapter, TimeoutDoesNotThrowWhenDoorClosed) {
  TimedDoor d(0);
  d.lock();
  DoorTimerAdapter adapter(d);
  EXPECT_NO_THROW(adapter.Timeout());
}

TEST(TimedDoor, RemainsOpenedAfterThrow) {
  TimedDoor d(0);
  d.lock();
  try {
    d.unlock();
  } catch (const std::runtime_error&) {
  }
  EXPECT_TRUE(d.isDoorOpened());
}
