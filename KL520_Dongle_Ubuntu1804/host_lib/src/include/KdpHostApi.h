/**
 * @file      KdpHostApi.h
 * @brief     kdp host api header file
 * @version   0.1 - 2019-04-18
 * @copyright (c) 2019 Kneron Inc. All right reserved.
 */

#ifndef __KDP_HOST_API_H__
#define __KDP_HOST_API_H__

#define MAX_COM_DEV    3

#include "kdp_host.h"
#include "kdp_host_async.h"


#if !defined(THREAD_USE_CPP11)

#include "pthread.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>


class mutex   
{  
public:  
    
  explicit mutex(bool processShared = false) :  
    _processShared(processShared)  
  {  
    pthread_mutexattr_t attr;  
    memset(&attr, 0, sizeof(pthread_mutexattr_t));
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);  
  
    if (processShared) {  
        int shared;  
        pthread_mutexattr_getpshared(&attr, &shared);  
        //assert(shared ==  PTHREAD_PROCESS_PRIVATE);  
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);  
    }  
     
    int err = pthread_mutex_init(&_mutex, &attr);  
    (void) err;  
  }  
  
  ~mutex()  
  {  
    int err = pthread_mutex_destroy(&_mutex);  
    (void) err;  
  }  
  
  int lock()  
  {  
    return pthread_mutex_lock(&_mutex);  
  }  
  
  int unlock()  
  {  
    return pthread_mutex_unlock(&_mutex);  
  }  
  
  pthread_mutex_t* getmutexPtr()  
  {  
    return &_mutex;  
  }  
  
private:  
  pthread_mutex_t _mutex;  
  bool _processShared;  
  //mutex(const mutex&) ;  //= delete
  //mutex& operator=(const mutex&) ;  //= delete
};

/*mutexLockGuard holds this_mutex; 
  uses constructors and destructors in C++ to apply for and release a mutex
*/
class mutexLockGuard   
{  
public:  
  mutexLockGuard(const mutexLockGuard&) ;  //= delete
  mutexLockGuard& operator=(const mutexLockGuard&) ;  //= delete
  
  explicit mutexLockGuard(mutex& mutex) :  
   _mutex(mutex)  
  {  
    _mutex.lock();  
  }  
  
  ~mutexLockGuard()  
  {  
    _mutex.unlock();  
  }  
  
private:  
  mutex& _mutex;  
};

class Condition 
{
    public:
       explicit  Condition()
        {
          pthread_cond_init(&cond_,NULL);
        }

        ~Condition()
        {
          pthread_cond_destroy(&cond_);
        }

        //Encapsulation pthread_cond_wait
        /*
        void wait()
        {
          pthread_cond_wait(&cond_, mutex_.getmutexPtr());
        }*/
                
        void wait_for(mutex& _mutex,int timeout_ms)
        {
          struct timespec abstime;
          struct timeval now;
         
          gettimeofday(&now, NULL);
          long nsec = now.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
          abstime.tv_sec=now.tv_sec + nsec / 1000000000 + timeout_ms / 1000;
          abstime.tv_nsec=nsec % 1000000000;
          pthread_cond_timedwait(&cond_, _mutex.getmutexPtr(), &abstime);
        }
        
        //Encapsulation pthread_cond_signal
        void notify()
        {
          pthread_cond_signal(&cond_);
        }
        
        //Encapsulation pthread_cond_broadcast
        void notify_all()
        {
          pthread_cond_broadcast(&cond_);
        }

    private:
        pthread_cond_t cond_;
        //mutex& mutex_;
};

#endif

typedef void *khost_device_t;
khost_device_t kdp_dev_to_khost_dev(int dev_idx);
int kdp_khost_dev_to_dev_idx(khost_device_t dev);

#endif

