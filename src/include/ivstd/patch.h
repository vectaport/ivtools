#ifndef _patch_h
#define _patch_h

/* PATCH_KEY: an identifier for the most recently applied patch, shown on
   each editor's startup banner (build_stamp(), ComUtil/util.c) so a
   running binary proves which patch built it.  Shared by all five editor
   main.c's (comdraw, drawserv_, comterp_, flipbook, graphdraw) rather than
   tracked independently per binary, since a single patch commonly touches
   more than one of them at once -- bump it here and every binary's banner
   picks it up.

   "Patch" (not "commit") is the deliberately VCS-agnostic term -- see
   Larry Wall's `patch` and the general Unix sense of a named unit of
   change.  PATCH_KEY is committed as a plain literal, not derived from
   git, so it never runs into a file trying to contain the hash of the
   commit it's part of.  Traceability back to an exact commit comes from a
   matching git tag: after bumping this value and committing, push a tag
   with the same name pointing at that commit (`git tag <key> && git push
   origin <key>`) -- GitHub (and git itself) resolve tags and commit SHAs
   through the same ref-lookup mechanism, so anyone can look up a
   PATCH_KEY value later and land on the exact commit it names. */
#define PATCH_KEY "aaccee00"

#endif /* _patch_h */
