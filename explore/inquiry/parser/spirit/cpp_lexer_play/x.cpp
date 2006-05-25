{
}

SOAP_FMAC1 int SOAP_FMAC2 soap_putheader(struct soap *soap)
{
        if (soap->header)
        {       soap->is_in_header = 1;
                soap_out_SOAP_ENV__Header(soap, "SOAP-ENV:Header", 0, soap->header, NULL);
                soap->is_in_header = 0;
        }
        return SOAP_OK;
}

