/*
This file is part of MAE.

History:

[2006.03.21 - Helio Perroni Filho] Created. 
*/

#ifdef __cplusplus
extern "C"
{
#endif

void mae_logging_trace(const char* call);

void mae_logging_trace2(const char* call, const char* arglist, ...);

#ifdef __cplusplus
}
#endif
