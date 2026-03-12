# P1 Setup: Command, Orders, Input

After building, enable P1 by changing **only** the Player Controller and Default Pawn on your existing GameMode. Do **not** change the Game Mode class if that causes a crash.

## Recommended: Use your existing Blueprint GameMode

- Open your **Blueprint** GameMode (e.g. **BP_RTSGameMode**) in the editor.
- In Class Defaults set **Player Controller Class** → `RTSPlayerController` and **Default Pawn Class** → `RTSCameraPawn`.
- Save. You do **not** need to change "Default GameMode" in Project Settings.

## Optional: C++ GameMode

- Only if you prefer it: set **Default GameMode** to `RTSGameModeBase` in Project Settings. If the editor crashes when selecting it, keep using the Blueprint method above.

## Input (legacy)

- **LMB**: Select unit under cursor (replaces selection).
- **Shift+LMB**: Add unit under cursor to selection.
- **RMB**: Context order — ground = Move; enemy unit = Attack; same-faction unit = Move to that unit’s location. Only units in the **selected Hero’s command radius** (2500 UU) receive the new order.
- **WASD / Arrows**: Camera pan only (no unit control).

## Data

- Add **PostCombatBehavior** column to your Units DataTable (`FUnitRow::PostCombatBehavior`: Advance / Hold / Retreat). Default in code is Hold.

## Collision

- Cursor trace uses **Visibility** channel. Ensure unit/ground meshes block Visibility so LMB/RMB hit the right targets.
