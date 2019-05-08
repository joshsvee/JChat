// CriticalSection.cpp : Provides mutual exclusion for shared data across threads. (Jared)

#include "sys_win.h"
#include "criticalsection.h"

// Default Constructor.
CriticalSection::CriticalSection(void)
{
    InitializeCriticalSection(&m_cs);
}

// Destructor.
// Ensures deletion of the critical section member.
CriticalSection::~CriticalSection(void)
{
    DeleteCriticalSection(&m_cs);
}

// Gain access to the critical section object.
// Enter is a blocking call that will not return until the critical 
// section object is signaled (becomes available).
void CriticalSection::Enter(void)
{
    EnterCriticalSection(&m_cs);
}

// Releases the CriticalSection object for use by another thread.
void CriticalSection::Leave(void)
{
    LeaveCriticalSection(&m_cs);
}
