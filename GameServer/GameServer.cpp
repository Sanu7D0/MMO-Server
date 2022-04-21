#include "pch.h"
#include "CorePch.h"
#include "CoreMacro.h"
#include "ThreadManager.h"
#include "Memory.h"

#include <atomic>
#include <mutex>

class RoadRuneer {
public:
    int32 madness;
public:
	RoadRuneer() {}
    RoadRuneer(int32 madness) : madness(madness) { }
    ~RoadRuneer() { }
};

int main()
{
    RoadRuneer* runner = xnew<RoadRuneer>(50);
    xdelete(runner);

    xmap<int32, RoadRuneer> m;
    m[100] = RoadRuneer();
    std::cout << 1;
}