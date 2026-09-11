#ifndef _patch_h
#define _patch_h

/* PATCH_KEY: an identifier for the most recently applied patch, shown on
   each editor's startup banner (build_stamp(), ComUtil/util.c) so a
   running binary proves which patch built it.  Shared by all five editor
   main.c's (comdraw, drawserv_, comterp_, flipbook, graphdraw) rather than
   tracked independently per binary -- bump it here and every binary's
   banner picks it up.

   "Patch" (not "commit") is the deliberately VCS-agnostic term -- see
   Larry Wall's `patch`.  To bump it: change the value below and commit,
   nothing more.

   TAGGING IS AUTOMATIC, DONE BY MERGE CI, NOT BY WHOEVER BUMPS THIS
   VALUE. Do not run `git tag`/`git push origin <tag>` yourself -- CI
   tags the merge commit on the default branch with this exact value once
   the change lands, which is what makes a PATCH_KEY later resolve back
   to the commit it named. A PR that only bumps this value needs no
   separate tagging step and no tag-push commit. */
#define PATCH_KEY "4223a1af"

#endif /* _patch_h */
