void Mouse_SetPosition(signed int X, signed int Y);

signed int Mouse_GetX(void);
signed int Mouse_GetY(void);
/* bit 0 is left, bit 1 is right, bit 2 is middle */
int Mouse_GetButtons(void);
void Mouse_SetButtons(int nButton, BOOL bState);

