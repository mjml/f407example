#include <stdint.h>

struct Chirp
{
	struct State
	{
		Chirp* chirp;
		uint16_t  phase;
		uint32_t  stoptick;
	};

	uint16_t freq1;
	uint16_t freq2;
	uint16_t intensity1;
	uint16_t intensity2;
	uint32_t duration; // microseconds

    static uint16_t sina[256];
    
	
	Chirp (int f, int d) : freq1(f), freq2(f), duration(d) {}
	Chirp (int f1, int f2, int d) : freq1(f1), freq2(f2), duration(d) {}
	~Chirp () {}

    /**
     * Called at the start of service.
     **/
    static void Initialize ();

    /**
     * In theory, called at the end of service. Is currently nooped.
     **/
    static void Finalize ();

    /**
    * Called from an ISR
    * */
    static void Service ();

    /**
     * Helper constructor
     **/
	static Chirp Sec(int f, int s) { return { .freq1 = f, .freq2 = f, .d = s * 1000 * 1000}; }

	State Instantiate () {
        State s;
        s.chirp = this;
        s.phase = 0;
        // s.stoptick = ?
        return s;
	}

};
