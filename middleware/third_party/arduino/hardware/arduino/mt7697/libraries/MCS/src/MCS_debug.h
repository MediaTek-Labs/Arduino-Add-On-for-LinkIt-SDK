#ifndef MCS_DEBUG_H
#define MCS_DEBUG_H

/* ----------------------------------------------------------------------------
Debug Switch
---------------------------------------------------------------------------- */

#if 0   // 1 to enable debug log output
#define _DEBUG_PRINT(x)  Serial.println(x)
#else
#define _DEBUG_PRINT(x)  
#endif

/* ----------------------------------------------------------------------------
Profiling Switch
---------------------------------------------------------------------------- */

#if 0   // 1 to enable profiling 
class _profileObj
{
public:
    _profileObj(const String& funcname):
    mStart(millis()),
    mName(funcname)
    {
    }
    ~_profileObj() {
        stop();
    }

    void stop() {
        if(mStart > 0)
        {
            unsigned long d = millis() - mStart;
            mStart = 0;
            Serial.println(String("[profile]")+mName+String(" cost ")+String(d)+String("ms!"));
        }
    }

private:
    unsigned long mStart;
    String mName;
};
#define _PROF_START(x)  _profileObj _##x##_obj(#x)
#define _PROF_END(x)    _##x##_obj.stop()
#else
#define _PROF_START(x)  
#define _PROF_END(x)        
#endif

#endif