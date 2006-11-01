#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "../stemdtch/stemdtch.h"


MODULE = echoclient		PACKAGE = echoclient		

void send_message( msg, len )
        const char *msg
        unsigned    len
        CODE:
          _send_msg( msg, len );

void wait_stem()
        CODE:
          _wait_stem();
