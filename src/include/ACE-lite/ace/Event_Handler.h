/* ACE-lite (#147) compatibility shim: lets unedited consumer code that says
   #include <ace/Event_Handler.h> resolve to the bundled subset when src/include/ACE-lite
   is on the include path (in-tree build only).  The external --with-ace build
   adds -isystem $(ACEDIR) and never puts this dir on the path, so real libACE
   wins there. */
#include <ACE-lite/Event_Handler.h>
