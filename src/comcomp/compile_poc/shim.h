/* Flat C ABI in front of ComTerpServ, so Go (via cgo, which only
   understands C) can drive a real ComTerp instance without linking
   against its C++ classes directly. */

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
   comterp_bridge_free(). */
const char* comterp_bridge_eval(comterp_handle h, const char* expr);

const char* comterp_bridge_errmsg(comterp_handle h);

#ifdef __cplusplus
}
#endif
