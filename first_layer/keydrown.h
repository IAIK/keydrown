// inject mouse events
#define INJECT_MOUSE 0
// inject keyboard events
#define INJECT_KB 1

// maximum random delay between fake keystrokes (ns)
#define RND_MAX 20000000UL //~20ms
// minimum random delay (ns)
#define RND_MIN 0UL // 0ms

// maximum random delay of keyboard ISR
#define ISR_DELAY_MAX 100UL //~100µs
#define ISR_DELAY_MIN 0UL   // 0µs

// prefetch the handler to prevent prime+probe attacks
#define PREFETCH_HANDLER 1

// set to higher value for easier debugging
#define MIN_TIMER_SECONDS 0

// procfs
#define KEYDROWN_PROCFS_ENABLE "keydrown"
#define KEYDROWN_MISCFS_KEYS_REAL "keydrown_real"
#define KEYDROWN_MISCFS_KEYS_FAKE "keydrown_fake"

// state
#define STATE_INJECT 0x1

// -------------------------------------

#define MOD "keydrown: "
