#pragma once
#include "interception.h"

void set_hook_enabled(bool on);
void stop_hook_now_for_exit();
void inject_prefix(InterceptionContext ctx, InterceptionDevice dev);
void inject_suffix(InterceptionContext ctx, InterceptionDevice dev);
void hook_thread_main();
