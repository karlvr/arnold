#import <Cocoa/Cocoa.h>

void *GetWindowForView(void *pWindowHandle)
{
NSView *view = (NSView *)pWindowHandle;
printf("View: %p\n",view);
NSWindow *window = (NSWindow *)[view window];
printf("window: %p\n", window);
return (void *)window;
}
