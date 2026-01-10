#include "raylib.h"
#include <stdio.h>

#define SCREEN_WIDTH 16 * 30
#define SCREEN_HEIGHT 16 * 30 + 50

int
main (void)
{
   InitWindow (SCREEN_WIDTH, SCREEN_HEIGHT, "Minesweeper -- vs-123");

   while (!WindowShouldClose ())
      {
         if (IsKeyPressed (KEY_ESCAPE))
            {
               goto die;
            }
         BeginDrawing ();
         ClearBackground (RAYWHITE);
         EndDrawing ();
      }

die:
   CloseWindow ();

   return 0;
}
