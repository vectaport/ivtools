/* Flat C ABI over a live AttributeList, so Go (via cgo) can hold and
   mutate the same object ComTerp itself reads and writes, proved
   standalone here before any comcomp/funccompile.comt wiring. Values
   are string-only for this spike; other AttributeValue types are
   future work. */

#ifdef __cplusplus
extern "C" {
#endif

typedef void* proplist_handle;

proplist_handle proplist_new(void);
void proplist_free(proplist_handle h);

void proplist_set(proplist_handle h, const char* key, const char* val);
/* Returned pointer is owned by the handle, valid until the next
   proplist_* call on it or proplist_free(). *found is set 0/1. */
const char* proplist_get(proplist_handle h, const char* key, int* found);
int proplist_has(proplist_handle h, const char* key);
int proplist_delete(proplist_handle h, const char* key);
int proplist_len(proplist_handle h);
/* nil once i >= proplist_len(h). */
const char* proplist_key_at(proplist_handle h, int i);
int proplist_touch(proplist_handle h, const char* key);

/* Renders via AttributeList's own serialize(), proving ComTerp and Go
   are looking at the same object, not a translated copy. */
const char* proplist_print(proplist_handle h);

#ifdef __cplusplus
}
#endif
