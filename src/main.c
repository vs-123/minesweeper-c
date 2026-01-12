#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"
#define YSTAR_IMPLEMENTATION
#include "ystar.h"

#define NOT_IMPL assert (0 && "NOT IMPLEMENTED");

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef uint8_t u8;
/*typedef uint64_t u64;*/

typedef struct
{
   bool is_mine;
   bool is_revealed;
   bool has_flag;
   u8 adjacent_mines_count;
   /* won't ever exceed 9 so... u8 seems good enough */
} cell_t;

typedef struct
{
   u8 grid_columns;
   u8 grid_rows;
   u8 cell_size;
   u8 mines_count;
   u8 ui_height;
   u64 seed;

   int screen_width;
   int screen_height;

   cell_t *cell_grid;

   float start_time;
   float end_time;
   float pause_duration;
   float confirm_start_time;

   bool has_game_ended;
   bool has_user_won;
   bool is_reset_confirmed;
   bool is_first_click;

   Texture2D mine_texture;
   Texture2D flag_texture;

   Rectangle btn_new_game;
   Rectangle btn_yes;
   Rectangle btn_no;
   Rectangle modal_rect;

   /* colour palette for the numbers */
   Color colour_palette[9];
} mswpr_t;

/* DECLARATIONS */

mswpr_t mswpr_init (void);
void mswpr_free (mswpr_t *mswpr);

void mswpr_reset (mswpr_t *mswpr);

void mswpr_calc_adj_mines (mswpr_t *mswpr);
void mswpr_place_mines (mswpr_t *mswpr);

void mswpr_update (mswpr_t *mswpr);

void mswpr_draw_end_screen (mswpr_t *mswpr);
void mswpr_draw_reset_modal (mswpr_t *mswpr);
void mswpr_draw_ui (mswpr_t *mswpr);
void mswpr_draw (mswpr_t *mswpr);

void mswpr_run (mswpr_t *mswpr);

void mswpr_first_click_safe_zone (mswpr_t *mswpr, int cell, int col);
void mswpr_reveal_cell (mswpr_t *mswpr, int cell_row, int cell_col);
void mswpr_auto_reveal (mswpr_t *mswpr, int cell_row, int cell_col);

int
main (void)
{
   mswpr_t mswpr = mswpr_init ();
   mswpr_run (&mswpr);
   mswpr_free (&mswpr);

   return 0;
}

mswpr_t
mswpr_init (void)
{
   mswpr_t mswpr;

   mswpr.grid_columns = 15;
   mswpr.grid_rows    = 15;
   mswpr.cell_size    = 35;
   mswpr.mines_count  = 1;
   mswpr.ui_height    = 50;
   mswpr.seed         = 1;

   mswpr.screen_width  = mswpr.grid_columns * mswpr.cell_size;
   mswpr.screen_height = (mswpr.grid_rows * mswpr.cell_size) + mswpr.ui_height;

   InitWindow (mswpr.screen_width, mswpr.screen_height,
               "Minesweeper -- vs-123");

   mswpr.cell_grid
       = malloc (sizeof (cell_t) * mswpr.grid_rows * mswpr.grid_columns);

   mswpr.start_time         = GetTime ();
   mswpr.end_time           = 0.0;
   mswpr.pause_duration     = 0.0;
   mswpr.confirm_start_time = 0.0;

   mswpr.has_game_ended     = false;
   mswpr.has_user_won       = false;
   mswpr.is_reset_confirmed = false;
   mswpr.is_first_click     = true;

   mswpr.btn_new_game
       = (Rectangle){ .x = 10, .y = 10, .width = 105, .height = 30 };

   int modal_width   = 300;
   int modal_height  = 150;
   int screen_width  = mswpr.grid_columns * mswpr.cell_size;
   int screen_height = mswpr.grid_rows * mswpr.cell_size + mswpr.ui_height;

   mswpr.modal_rect
       = (Rectangle){ .x      = (float)(screen_width / 2 - modal_width / 2),
                      .y      = (float)(screen_height / 2 - modal_height / 2),
                      .width  = (float)modal_width,
                      .height = (float)modal_height };

   mswpr.btn_yes = (Rectangle){ .x = mswpr.modal_rect.x + 30,
                                .y = mswpr.modal_rect.y + modal_height - 50,
                                .width  = 100,
                                .height = 30 };

   mswpr.btn_no = (Rectangle){ .x     = mswpr.modal_rect.x + modal_width - 130,
                               .y     = mswpr.modal_rect.y + modal_height - 50,
                               .width = 100,
                               .height = 30 };

   mswpr.mine_texture = LoadTexture ("assets/mine.png");
   mswpr.flag_texture = LoadTexture ("assets/flag.png");

   mswpr.colour_palette[0] = WHITE;
   mswpr.colour_palette[1] = (Color){ 0, 70, 241, 255 };
   mswpr.colour_palette[2] = DARKGREEN;
   mswpr.colour_palette[3] = RED;
   mswpr.colour_palette[4] = DARKBLUE;
   mswpr.colour_palette[5] = ORANGE;
   mswpr.colour_palette[6] = (Color){ 64, 224, 208, 255 };
   mswpr.colour_palette[7] = BLACK;
   mswpr.colour_palette[8] = GRAY;

   Image window_icon = LoadImage ("assets/mine.png");
   SetWindowIcon (window_icon);
   UnloadImage (window_icon);

   mswpr_reset (&mswpr);

   return mswpr;
}

void
mswpr_free (mswpr_t *mswpr)
{
   if (mswpr->cell_grid != NULL)
      {
         free (mswpr->cell_grid);
         mswpr->cell_grid = NULL;
      }

   if (mswpr->mine_texture.id > 0)
      {
         UnloadTexture (mswpr->mine_texture);
      }
   if (mswpr->flag_texture.id > 0)
      {
         UnloadTexture (mswpr->flag_texture);
      }

   CloseWindow ();
}

void
mswpr_reset (mswpr_t *mswpr)
{
   mswpr->has_game_ended = false;
   mswpr->has_user_won   = false;
   mswpr->start_time     = GetTime ();
   mswpr->end_time       = 0.0;
   mswpr->pause_duration = 0.0;
   mswpr->is_first_click = true;
   mswpr->seed           = time (NULL) ^ (u64)mswpr_reset;
   mswpr->mines_count    = ystar_between (&mswpr->seed, 20, 40);

   /* don't reallocate, just reset the grid */
   int total_cells = mswpr->grid_rows * mswpr->grid_columns;
   for (int i = 0; i < total_cells; i++)
      {
         mswpr->cell_grid[i] = (cell_t){ .is_mine              = false,
                                         .is_revealed          = false,
                                         .has_flag             = false,
                                         .adjacent_mines_count = 0 };
      }

   mswpr_place_mines (mswpr);
   mswpr_calc_adj_mines (mswpr);
}

void
mswpr_place_mines (mswpr_t *mswpr)
{
   int mines_placed = 0;

   while (mines_placed < mswpr->mines_count)
      {
         int row = ystar (&mswpr->seed) % mswpr->grid_rows;
         int col = ystar (&mswpr->seed) % mswpr->grid_columns;

         cell_t *current_cell
             = &mswpr->cell_grid[row * mswpr->grid_columns + col];

         if (!current_cell->is_mine)
            {
               current_cell->is_mine = true;
               mines_placed++;
            }
      }
}

void
mswpr_calc_adj_mines (mswpr_t *mswpr)
{
   for (int row = 0; row < mswpr->grid_rows; row++)
      {
         for (int col = 0; col < mswpr->grid_columns; col++)
            {
               cell_t *current_cell
                   = &mswpr->cell_grid[row * mswpr->grid_columns + col];

               if (current_cell->is_mine)
                  {
                     continue;
                  }

               int count = 0;

               for (int i = -1; i <= 1; i++)
                  {
                     for (int j = -1; j <= 1; j++)
                        {
                           int new_row = row + i;
                           int new_col = col + j;

                           if (new_row >= 0 && new_row < mswpr->grid_rows
                               && new_col >= 0
                               && new_col < mswpr->grid_columns)
                              {
                                 cell_t *neighbor
                                     = &mswpr->cell_grid
                                            [new_row * mswpr->grid_columns
                                             + new_col];
                                 if (neighbor->is_mine)
                                    {
                                       count++;
                                    }
                              }
                        }
                  }

               current_cell->adjacent_mines_count = count;
            }
      }
}

void
mswpr_update (mswpr_t *mswpr)
{
   Vector2 mouse_position = GetMousePosition ();

   if (mswpr->is_reset_confirmed)
      {
         if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
            {
               if (CheckCollisionPointRec (mouse_position, mswpr->btn_yes))
                  {
                     mswpr_reset (mswpr);
                     mswpr->end_time           = 0.0;
                     mswpr->is_reset_confirmed = false;
                  }
               else if (CheckCollisionPointRec (mouse_position, mswpr->btn_no))
                  {
                     mswpr->pause_duration
                         += GetTime () - mswpr->confirm_start_time;
                     mswpr->is_reset_confirmed = false;
                  }
            }
      }
   else
      {
         if (!mswpr->has_game_ended && !mswpr->has_user_won)
            {
               if (mouse_position.y > mswpr->ui_height)
                  {
                     int cell_col = (int)mouse_position.x / mswpr->cell_size;
                     int cell_row = (int)(mouse_position.y - mswpr->ui_height)
                                    / mswpr->cell_size;

                     if (cell_row >= 0 && cell_row < mswpr->grid_rows
                         && cell_col >= 0 && cell_col < mswpr->grid_columns)
                        {
                           cell_t *current_cell
                               = &mswpr->cell_grid[cell_row
                                                       * mswpr->grid_columns
                                                   + cell_col];

                           if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_F))
                              {
                                 if (mswpr->is_first_click)
                                    {
                                       mswpr_first_click_safe_zone (
                                           mswpr, cell_row, cell_col);
                                       mswpr->is_first_click = false;
                                       current_cell
                                           = &mswpr->cell_grid
                                                  [cell_row
                                                       * mswpr->grid_columns
                                                   + cell_col];
                                    }

                                 if (!current_cell->has_flag)
                                    mswpr_reveal_cell (mswpr, cell_row,
                                                       cell_col);
                              }
                           else if (IsMouseButtonPressed (MOUSE_RIGHT_BUTTON) || IsKeyPressed(KEY_SPACE))
                              {
                                 if (!current_cell->is_revealed)
                                    current_cell->has_flag
                                        = !current_cell->has_flag;
                              }
                           else if (IsMouseButtonPressed (MOUSE_MIDDLE_BUTTON) || IsKeyPressed(KEY_E))
                              {
                                 if (current_cell->is_revealed
                                     && current_cell->adjacent_mines_count > 0)
                                    mswpr_auto_reveal (mswpr, cell_row,
                                                       cell_col);
                              }
                        }
                  }

               if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON)
                   && CheckCollisionPointRec (mouse_position,
                                              mswpr->btn_new_game))
                  {
                     mswpr->is_reset_confirmed = true;
                     mswpr->confirm_start_time = GetTime ();
                  }
            }
         else if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON)
                  && CheckCollisionPointRec (mouse_position,
                                             mswpr->btn_new_game))
            {
               mswpr_reset (mswpr);
               mswpr->end_time = 0.0;
            }
      }

   int total_cells    = mswpr->grid_rows * mswpr->grid_columns;
   int revealed_count = 0;
   for (int i = 0; i < total_cells; i++)
      {
         if (mswpr->cell_grid[i].is_revealed)
            revealed_count++;
      }

   if (revealed_count == total_cells - mswpr->mines_count)
      mswpr->has_user_won = true;

   if ((mswpr->has_game_ended || mswpr->has_user_won)
       && mswpr->end_time == 0.0)
      {
         mswpr->end_time
             = GetTime () - mswpr->start_time - mswpr->pause_duration;
      }
}

void
mswpr_draw_cell (mswpr_t *mswpr, int row, int col)
{
   int x = col * mswpr->cell_size;
   int y = row * mswpr->cell_size + mswpr->ui_height;

   Rectangle rect = { (float)x, (float)y, (float)mswpr->cell_size,
                      (float)mswpr->cell_size };
   cell_t cell    = mswpr->cell_grid[row * mswpr->grid_columns + col];

   if (cell.is_revealed)
      {
         DrawRectangleRec (rect, LIGHTGRAY);
      }
   else
      {
         DrawRectangleRec (rect, GRAY);
      }

   if (cell.is_revealed)
      {
         if (cell.is_mine)
            {
               if (mswpr->mine_texture.id > 0)
                  {
                     float scale
                         = (float)mswpr->cell_size / mswpr->mine_texture.width;
                     DrawTextureEx (mswpr->mine_texture,
                                    (Vector2){ (float)x, (float)y }, 0.0f,
                                    scale, WHITE);
                  }
               else
                  {
                     DrawText ("M", x + mswpr->cell_size / 4,
                               y + mswpr->cell_size / 4, 20, BLACK);
                  }
            }
         else if (cell.adjacent_mines_count > 0)
            {
               int num          = cell.adjacent_mines_count;
               const char *text = TextFormat ("%d", num);
               int centerX
                   = x + (mswpr->cell_size - MeasureText (text, 20)) / 2;
               int centerY = y + (mswpr->cell_size - 20) / 2;
               DrawText (text, centerX, centerY, 20,
                         mswpr->colour_palette[num]);
            }
      }
   else if (cell.has_flag)
      {
         if (mswpr->flag_texture.id > 0)
            {
               float scale
                   = (float)mswpr->cell_size / mswpr->flag_texture.width;
               DrawTextureEx (mswpr->flag_texture,
                              (Vector2){ (float)x, (float)y }, 0.0f, scale,
                              WHITE);
            }
         else
            {
               DrawText ("F", x + mswpr->cell_size / 3,
                         y + mswpr->cell_size / 4, 20, MAROON);
            }
      }

   DrawRectangleLinesEx (rect, 1, BLACK);
}

void
mswpr_draw_ui (mswpr_t *mswpr)
{
   DrawRectangleRec (mswpr->btn_new_game, LIGHTGRAY);
   DrawRectangleLinesEx (mswpr->btn_new_game, 2, GRAY);

   const char *btn_text = "New Game";
   int btn_font_size    = 18;
   int btn_text_w       = MeasureText (btn_text, btn_font_size);

   DrawText (btn_text,
             mswpr->btn_new_game.x + (mswpr->btn_new_game.width / 2)
                 - (btn_text_w / 2),
             mswpr->btn_new_game.y + (mswpr->btn_new_game.height / 2)
                 - (btn_font_size / 2),
             btn_font_size, BLACK);

   int flags_placed = 0;
   int total_cells  = mswpr->grid_rows * mswpr->grid_columns;
   for (int i = 0; i < total_cells; i++)
      {
         if (mswpr->cell_grid[i].has_flag)
            flags_placed++;
      }
   int mines_rem = mswpr->mines_count - flags_placed;

   float elapsed
       = (mswpr->has_game_ended || mswpr->has_user_won)
             ? mswpr->end_time
             : (GetTime () - mswpr->start_time - mswpr->pause_duration);

   const char *timer_txt = TextFormat ("TIME: %ds", (int)elapsed);
   const char *mine_txt  = TextFormat ("MINES: %d", mines_rem);

   int stats_x = mswpr->btn_new_game.x + mswpr->btn_new_game.width + 20;
   int stats_y = mswpr->btn_new_game.y + 7;

   DrawText (timer_txt, stats_x, stats_y, 20,
             (int)elapsed > 999 ? RED : BLACK);
   DrawText (mine_txt, stats_x + 130, stats_y, 20, MAROON);

   DrawLine (0, mswpr->ui_height - 1, mswpr->screen_width,
             mswpr->ui_height - 1, DARKGRAY);
}

void
mswpr_draw (mswpr_t *mswpr)
{
   BeginDrawing ();
   ClearBackground (RAYWHITE);

   mswpr_draw_ui (mswpr);

   for (int row = 0; row < mswpr->grid_rows; row++)
      {
         for (int col = 0; col < mswpr->grid_columns; col++)
            {
               mswpr_draw_cell (mswpr, row, col);
            }
      }

   if (mswpr->has_game_ended || mswpr->has_user_won)
      {
         mswpr_draw_end_screen (mswpr);
      }

   if (mswpr->is_reset_confirmed)
      {
         mswpr_draw_reset_modal (mswpr);
      }

   EndDrawing ();
}

void
mswpr_draw_end_screen (mswpr_t *mswpr)
{
   int grid_y = mswpr->ui_height;
   int grid_h = mswpr->screen_height - mswpr->ui_height;

   DrawRectangle (0, grid_y, mswpr->screen_width, grid_h,
                  Fade (LIGHTGRAY, 0.8f));

   const char *text = mswpr->has_user_won ? "YOU WIN!" : "GAME OVER";
   Color color      = mswpr->has_user_won ? DARKGREEN : RED;

   int font_size = 40;
   int text_w    = MeasureText (text, font_size);
   int text_x    = (mswpr->screen_width / 2) - (text_w / 2);
   int text_y    = grid_y + (grid_h / 2) - (font_size / 2);

   DrawText (text, text_x + 2, text_y + 2, font_size, Fade (BLACK, 0.3f));
   DrawText (text, text_x, text_y, font_size, color);
}

void
mswpr_draw_reset_modal (mswpr_t *mswpr)
{
   DrawRectangle (0, 0, mswpr->screen_width, mswpr->screen_height,
                  Fade (BLACK, 0.5f));

   DrawRectangleRec (mswpr->modal_rect, RAYWHITE);
   DrawRectangleLinesEx (mswpr->modal_rect, 2, BLACK);

   const char *msg = "Start a new game?";
   int msg_w       = MeasureText (msg, 20);
   DrawText (msg,
             mswpr->modal_rect.x + (mswpr->modal_rect.width / 2) - (msg_w / 2),
             mswpr->modal_rect.y + 30, 20, BLACK);

   DrawRectangleRec (mswpr->btn_yes, GREEN);
   DrawRectangleLinesEx (mswpr->btn_yes, 1, BLACK);
   int yes_w = MeasureText ("YES", 20);
   DrawText ("YES",
             mswpr->btn_yes.x + (mswpr->btn_yes.width / 2) - (yes_w / 2),
             mswpr->btn_yes.y + 5, 20, WHITE);

   DrawRectangleRec (mswpr->btn_no, RED);
   DrawRectangleLinesEx (mswpr->btn_no, 1, BLACK);
   int no_w = MeasureText ("NO", 20);
   DrawText ("NO", mswpr->btn_no.x + (mswpr->btn_no.width / 2) - (no_w / 2),
             mswpr->btn_no.y + 5, 20, WHITE);
}

void
mswpr_run (mswpr_t *mswpr)
{
   while (!WindowShouldClose ())
      {
         mswpr_update (mswpr);
         mswpr_draw (mswpr);
      }
}

void
mswpr_first_click_safe_zone (mswpr_t *mswpr, int safe_row, int safe_col)
{
   int min_row = MAX (0, safe_row - 1);
   int max_row = MIN (mswpr->grid_rows - 1, safe_row + 1);
   int min_col = MAX (0, safe_col - 1);
   int max_col = MIN (mswpr->grid_columns - 1, safe_col + 1);

   for (int row = min_row; row <= max_row; row++)
      {
         for (int col = min_col; col <= max_col; col++)
            {
               cell_t *current_cell
                   = &mswpr->cell_grid[row * mswpr->grid_columns + col];

               if (current_cell->is_mine)
                  {
                     current_cell->is_mine = false;
                     bool placed           = false;

                     for (int r = 0; r < mswpr->grid_rows && !placed; r++)
                        {
                           for (int c = 0; c < mswpr->grid_columns && !placed;
                                c++)
                              {
                                 if (r < min_row || r > max_row || c < min_col
                                     || c > max_col)
                                    {
                                       cell_t *target
                                           = &mswpr->cell_grid
                                                  [r * mswpr->grid_columns
                                                   + c];
                                       if (!target->is_mine)
                                          {
                                             target->is_mine = true;
                                             placed          = true;
                                          }
                                    }
                              }
                        }
                  }
            }
      }

   mswpr_calc_adj_mines (mswpr);
}

void
mswpr_reveal_cell (mswpr_t *mswpr, int row, int col)
{
   if (row < 0 || row >= mswpr->grid_rows || col < 0
       || col >= mswpr->grid_columns)
      {
         return;
      }

   int index            = row * mswpr->grid_columns + col;
   cell_t *current_cell = &mswpr->cell_grid[index];

   if (current_cell->is_revealed || current_cell->has_flag)
      {
         return;
      }

   current_cell->is_revealed = true;

   if (current_cell->is_mine)
      {
         mswpr->has_game_ended = true;
         return;
      }

   if (current_cell->adjacent_mines_count > 0)
      {
         return;
      }

   for (int i = -1; i <= 1; i++)
      {
         for (int j = -1; j <= 1; j++)
            {
               if (i != 0 || j != 0)
                  {
                     mswpr_reveal_cell (mswpr, row + i, col + j);
                  }
            }
      }
}

void
mswpr_auto_reveal (mswpr_t *mswpr, int row, int col)
{
   cell_t *current_cell = &mswpr->cell_grid[row * mswpr->grid_columns + col];

   if (!current_cell->is_revealed || current_cell->is_mine)
      {
         return;
      }

   int num_flagged = 0;
   for (int i = -1; i <= 1; i++)
      {
         for (int j = -1; j <= 1; j++)
            {

               int nr = row + i;
               int nc = col + j;
               if (nr >= 0 && nr < mswpr->grid_rows && nc >= 0
                   && nc < mswpr->grid_columns)
                  {
                     if (mswpr->cell_grid[nr * mswpr->grid_columns + nc]
                             .has_flag)
                        num_flagged++;
                  }
            }
      }

   if (num_flagged == current_cell->adjacent_mines_count)
      {
         for (int i = -1; i <= 1; i++)
            {
               for (int j = -1; j <= 1; j++)
                  {
                     mswpr_reveal_cell (mswpr, row + i, col + j);
                  }
            }
      }
}
