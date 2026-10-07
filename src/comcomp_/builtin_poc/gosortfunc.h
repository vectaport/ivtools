// gosortfunc.h -- ComFunc wrapper for the ComTerp->Go direction of the
// comcomp boundary: registers the "gosort" built-in, which calls into
// Go code compiled (by funccompile.go, from the isort ComTerp func's own
// postfix(:tree)) and linked in as gosort/libgosort.a. See
// src/comcomp_/builtin_poc/README.md for the full pipeline.
#ifndef gosortfunc_h
#define gosortfunc_h

#include <ComTerp/comfunc.h>

class GoSortFunc : public ComFunc {
public:
    GoSortFunc(ComTerp*);
    virtual void execute();
};

#endif
