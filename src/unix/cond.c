// Copyright 2015 Apcera Inc. All rights reserved.

#include <errno.h>

#include "../natsp.h"
#include "../util.h"
#include "../mem.h"

natsStatus
natsCondition_Create(natsCondition **cond)
{
    natsCondition   *c = (natsCondition*) NATS_CALLOC(1, sizeof(natsCondition));
    natsStatus      s  = NATS_OK;

    if (c == NULL)
        return NATS_NO_MEMORY;

    if (pthread_cond_init(c, NULL) != 0)
        s = NATS_SYS_ERROR;

    if (s == NATS_OK)
        *cond = c;
    else
        NATS_FREE(c);

    return s;
}

void
natsCondition_Wait(natsCondition *cond, natsMutex *mutex)
{
    if (pthread_cond_wait(cond, mutex) != 0)
        abort();
}

static natsStatus
_timedWait(natsCondition *cond, natsMutex *mutex, bool isAbsolute, uint64_t timeout)
{
    int     r;
    struct  timespec ts;
    int64_t target = (isAbsolute ? timeout : (nats_Now() + timeout));

    ts.tv_sec = target / 1000;
    ts.tv_nsec = (target % 1000) * 1000000;

    if (ts.tv_nsec >= 1000000000L)
    {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }

    r = pthread_cond_timedwait(cond, mutex, &ts);

    if (r == 0)
        return NATS_OK;

    if (r == ETIMEDOUT)
        return NATS_TIMEOUT;

    abort();
    return NATS_SYS_ERROR;
}

natsStatus
natsCondition_TimedWait(natsCondition *cond, natsMutex *mutex, uint64_t timeout)
{
    return _timedWait(cond, mutex, false, timeout);
}

natsStatus
natsCondition_AbsoluteTimedWait(natsCondition *cond, natsMutex *mutex, uint64_t absoluteTime)
{
    return _timedWait(cond, mutex, true, absoluteTime);
}

void
natsCondition_Signal(natsCondition *cond)
{
    if (pthread_cond_signal(cond) != 0)
      abort();
}

void
natsCondition_Broadcast(natsCondition *cond)
{
    if (pthread_cond_broadcast(cond) != 0)
      abort();
}

void
natsCondition_Destroy(natsCondition *cond)
{
    if (cond == NULL)
        return;

    pthread_cond_destroy(cond);
    NATS_FREE(cond);
}