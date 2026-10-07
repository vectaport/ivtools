// host.cc -- a copy of comterp with the "gosort" built-in linked in
// statically (dynamic loading is future work, per Scott). Same
// ComTerpServ setup as bridge_poc/compile_poc's shim, plus one
// add_command() registering GoSortFunc. Runs the script named on argv[1].
#include "gosortfunc.h"

#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>

#include <cstdio>

int main(int argc, char** argv) {
    ComTerpServ* terp = new ComTerpServ(BUFSIZ * BUFSIZ);
    terp->add_defaults();
    terp->add_command("gosort", new GoSortFunc(terp));

    if (argc < 2) {
        fprintf(stderr, "usage: %s <script.comt>\n", argv[0]);
        return 1;
    }
    terp->runfile(argv[1]);
    if (*terp->errmsg()) {
        fprintf(stderr, "error: %s\n", terp->errmsg());
        return 1;
    }
    return 0;
}
