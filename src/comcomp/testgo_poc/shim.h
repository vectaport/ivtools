/* Flat C ABI in front of ComTerpServ, so Go (via cgo, which only
   understands C) can drive a real ComTerp instance without linking
   against its C++ classes directly. Same shim as bridge_poc/compile_poc;
   copied rather than shared so this POC builds standalone. */

#ifdef __cplusplus
extern "C" {
#endif

typedef void* comterp_handle;

comterp_handle comterp_bridge_new(void);
void comterp_bridge_free(comterp_handle h);

/* Evaluates one ComTerp expression and returns its printed, re-parseable
   form (the same text print(expr) would produce), or NULL on error --
   in which case comterp_bridge_errmsg(h) holds the reason. The returned
   string is owned by the handle and valid until the next eval call or
   comterp_bridge_free(). Each call runs against the same ComTerpServ
   instance, so a variable assigned in one call (e.g. "r=feed(...)") is
   still bound in the next -- this is what lets testgo build a ring once
   and feed/drain it over several separate eval calls. */
const char* comterp_bridge_eval(comterp_handle h, const char* expr);

const char* comterp_bridge_errmsg(comterp_handle h);

/* Direct access to a ring FIFO's backing memory, bypassing feed()/next()
   entirely -- the fast path discussed as a follow-up to the text-eval
   round trip above. head/tail/count/wrap are pointers into the ring's
   own live AttributeValues (AttributeValue::int_ref()'s storage), not
   copies: writing through them updates the same ints next()/feed() read,
   so Go and ComTerp agree on ring state without a round trip. Safe
   without a lock only under a single-writer guarantee (see shim.cc). */
typedef struct {
    char* buf;     /* base of the ring's backing bytes */
    int elemsz;    /* bytes per slot (1 for an untyped/byte ring) */
    int cap;       /* slots */
    int* head;
    int* tail;
    int* count;
    int* wrap;     /* nonzero = circular (the feed() default), 0 = :noring */
} comterp_ring_info;

/* Looks up 'name' as a top-level ComTerp variable (the same scope a bare
   "name=..." assignment writes into) and fills *out if it's a ring FIFO
   (as built by feed(string(n type))). Returns 1 on success, 0 if the name
   is unbound or isn't a ring. */
int comterp_bridge_ring_info(comterp_handle h, const char* name, comterp_ring_info* out);

/* Pack/unpack one AnyType ring slot's 40-byte chunk (ATTRVALUE_CHUNK_BYTES,
   attrvalue.h) for an int value -- the reverse of each other, matching
   ComValue::comval_encode()/comval_decode()'s own AnyType branch exactly,
   so a slot Go writes is indistinguishable from one feed() wrote, and
   vice versa. 'chunk' must point at exactly comterp_ring_info.elemsz bytes
   (40 for an AnyType ring). comterp_bridge_decode_int returns 0 if the
   chunk doesn't hold an IntType value.
*/
void comterp_bridge_encode_int(long val, char* chunk);
int comterp_bridge_decode_int(const char* chunk, long* out);

#ifdef __cplusplus
}
#endif
