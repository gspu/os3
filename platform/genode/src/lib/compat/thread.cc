/* threading interface */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/thread.h>

/* Genode includes */
#include <timer_session/connection.h>
#include <cpu_thread/client.h>

#include <genode_env.h>

/* local includes */
#include "thread.h"

extern "C" BOOL
ThreadEqual(l4_os3_thread_t one, l4_os3_thread_t two)
{
    return (one == two);
}

extern "C" BOOL
TaskEqual(l4_os3_thread_t one, l4_os3_thread_t two)
{
    return (one == two);
}

extern "C" void
ThreadSleep(unsigned long ms)
{
    Genode::Env &env = genode_env();
    try
    {
        static Timer::Connection timer { env };
        timer.msleep(ms);
    }
    catch (Genode::Service_denied)
    {
        io_log("Timer service not available!\n");
    };
}

extern "C" void
ThreadExit(void)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Thread *thread = Genode::Thread::myself();
    destroy(alloc, thread);
}

extern "C" void
TaskExit(int result)
{
    Genode::Env &env = genode_env();
    env.parent().exit(result);
}

extern "C" l4_os3_thread_t
ThreadMyself(void)
{
    Genode::Thread *thread = Genode::Thread::myself();
    return (l4_os3_thread_t)thread;
}

extern "C" APIRET
ThreadCreate(l4_os3_thread_t *thread, ThreadFunc fn, void *data, ULONG flags)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();

    *thread = (l4_os3_thread_t)new (alloc) OS2::Thread(env, fn, data, flags);
    return NO_ERROR;
}

extern "C" APIRET
ThreadCreateLong(l4_os3_thread_t *thread, ThreadFunc fn, void *data, ULONG flags,
                 const char *name, ULONG stacksize)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();

    *thread = (l4_os3_thread_t)new (alloc) OS2::Thread(env, fn, data,
                                                      flags, name,
                                                      stacksize);

    return NO_ERROR;
}

extern "C" void
ThreadKill(l4_os3_thread_t native)
{
    Genode::Env &env = genode_env();
    Genode::Thread *thread = (Genode::Thread *)native;
    env.cpu().kill_thread(thread->cap());
}

extern "C" void
ThreadSuspend(l4_os3_thread_t native, ULONG *eip, ULONG *esp)
{
    Genode::Thread *thread = (Genode::Thread *)native;
    Genode::Cpu_thread_client client(thread->cap());
    client.pause();
    *eip = client.state().ip;
    *esp = client.state().sp;
}

extern "C" void
ThreadResume(l4_os3_thread_t native, ULONG *eip, ULONG *esp)
{
    Genode::Thread *thread = (Genode::Thread *)native;
    Genode::Cpu_thread_client client(thread->cap());
    //client.start(esp, eip);
    client.resume();
}

extern "C" l4_os3_thread_t
ThreadWait(l4_os3_thread_t native)
{
    Genode::Thread *thread = (Genode::Thread *)native;
    thread->join();
    return thread;
}
