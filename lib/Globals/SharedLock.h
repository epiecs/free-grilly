#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Guards the Arduino Strings that are shared between tasks: the config:: Strings and the probe
// names and types. Assigning a String can free its buffer while another task is still copying it.
// Recursive, so a locked JsonUtilities function can call into GrillConfig that locks again.
// Never hold it while connecting, doing network I/O or delay(), copy the values to locals instead.

// A plain pointer without a constructor is set before any code runs, no thread-safe static init needed
inline SemaphoreHandle_t& shared_lock_handle(){
    static SemaphoreHandle_t handle = nullptr;
    return handle;
}

// Called once at the start of setup(), before any task is started
inline void create_shared_lock(){
    if(shared_lock_handle() == nullptr){
        shared_lock_handle() = xSemaphoreCreateRecursiveMutex();
    }
}

// Takes the lock for the current scope: SharedLock lock;
// Before create_shared_lock() it does nothing. Only the global constructors (Probe) and early
// setup() run then, when there are no other tasks yet.
class SharedLock {
    public:
        SharedLock() : handle(shared_lock_handle()) {
            if(handle != nullptr){ xSemaphoreTakeRecursive(handle, portMAX_DELAY); }
        }
        ~SharedLock() {
            if(handle != nullptr){ xSemaphoreGiveRecursive(handle); }
        }

        SharedLock(const SharedLock&) = delete;
        SharedLock& operator=(const SharedLock&) = delete;

    private:
        SemaphoreHandle_t handle;
};
