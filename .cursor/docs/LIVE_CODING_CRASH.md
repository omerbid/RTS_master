# Live Coding crash (EXCEPTION_ACCESS_VIOLATION)

## What happens

When you use **Live Coding** to compile and hot-reload, the editor can crash with:

- **Error**: `Unhandled Exception: EXCEPTION_ACCESS_VIOLATION 0x0000000300905a4d`
- **Location**: `dynamic initializer for 'StatPtr_STAT_CreateWidget'` in `UserWidget.h` (UMG)
- **Cause**: The game module is built as a **patch DLL**. When that DLL loads, it pulls in UMG’s `CreateWidget` code and runs static initializers (e.g. `STAT_CreateWidget`) in the patch context, which can access invalid memory.

This is triggered by C++ code that creates a **UUserWidget** with `CreateWidget<>()` (e.g. `RTSPlayerController` creating `URTSUnitInfoWidget`).

## What to do

**Use a normal build instead of Live Coding:**

1. In the Unreal Editor, use **Build → Build Solution** (or **Compile**), **not** “Live Coding”.
2. Or from command line, build without `-LiveCoding`:
   ```bat
   "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" -Target="RTS_MonstersEditor Win64 Development" -Project="C:/Users/shodi/Documents/Desktop/RTS_Monsters/RTS_Monsters.uproject"
   ```
3. After a full build, run the editor and PIE as usual. The widget and game code will run without this crash.

If you need hot-reload, do a full build after changing C++ that uses UMG (e.g. the unit info widget or player controller), then restart the editor or PIE.
