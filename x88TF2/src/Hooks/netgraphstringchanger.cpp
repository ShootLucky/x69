#include "../src/SDK/SDK.h"

MAKE_SIGNATURE(Q_snprintf, "client.dll", "4C 89 44 24 ? 4C 89 4C 24 ? 53 55 56 57 41 56 48 83 EC ? 49 8B D8 48 63 FA 48 8B F1 4C 8D 74 24 ? E8 ? ? ? ? 4C 89 74 24 ? 4C 8B CB 4C 8B C7", 0x0);
MAKE_HOOK(Q_snprintf, Signatures::Q_snprintf.Get(), int, __cdecl,
    char* buffer, int size, const char* format, ...)
{
    if (!format || !buffer)
        return -1;

    va_list args;
    va_start(args, format);

    // string do FPS
    if (strcmp(format, "fps:%4i   ping: %i ms") == 0)
    {
        int fps = va_arg(args, int);
        int ping = va_arg(args, int);
        va_end(args);

        // x69 em cima, fps embaixo
        return snprintf(
            buffer,
            size,
            "[phantom]\nfps:%4i   ping: %i ms",
            fps,
            ping
        );
    }

    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}