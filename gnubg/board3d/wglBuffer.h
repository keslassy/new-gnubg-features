
/*#define WGL_FRONT_COLOR_BUFFER_BIT_ARB 1 */
#define WGL_BACK_COLOR_BUFFER_BIT_ARB 2
#define WGL_DEPTH_BUFFER_BIT_ARB 4
/*#define WGL_STENCIL_BUFFER_BIT_ARB 8 */

extern int wglBufferInitialize(void);
extern HANDLE CreateBufferRegion(unsigned int buffers);
extern void SaveBufferRegion(HANDLE region, int x, int y, int width, int height);
extern void RestoreBufferRegion(HANDLE region, int x, int y, int width, int height);
/*void DeleteBufferRegion(HANDLE region); */
