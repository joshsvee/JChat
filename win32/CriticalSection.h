// CriticalSection.h : Provides mutual exclusion for shared data across threads. (Jared)

#ifndef _CRITICAL_SECTION_H_
#define _CRITICAL_SECTION_H_

// A synchronization object that allows one thread at a time to access a 
// resource or section of code. Critical sections are useful when ONLY ONE 
// thread at a time can be allowed to modify data or some other controlled 
// resource.
class CriticalSection
{
    private:

        // Critical section object that is used by all CriticalSection methods.
        // If a thread terminates while it has ownership of a critical section, 
        // the state of the critical section is undefined.
        // NOTE: A critical section object cannot be moved or copied. The process must also not 
        //       modify the object, but must treat it as logically opaque. 
        //       Use only the critical section functions to manage critical section objects.
        CRITICAL_SECTION m_cs;

        // Just in case someone tries to get crazy!
        CriticalSection(const CriticalSection &) { }
        CriticalSection &operator=(const CriticalSection &) { return *this; }

    public:

        // Default Constructor.
        CriticalSection(void);

        // Destructor.
        virtual ~CriticalSection(void);

        // Gain access to the critical section object.
        // Enter is a blocking call that will not return until the critical 
        // section object is signaled (becomes available).
        void Enter(void);

        // Releases the CriticalSection object for use by another thread.
        void Leave(void);
};

#endif // _CRITICAL_SECTION_H_
