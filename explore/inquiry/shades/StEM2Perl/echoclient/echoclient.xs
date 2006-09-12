#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "../stemdtch/stemdtch.h"


MODULE = echoclient		PACKAGE = echoclient		

void send_message( msg )
        const char *msg
        unsigned    len
        CODE:
          send_msg( msg, len );
