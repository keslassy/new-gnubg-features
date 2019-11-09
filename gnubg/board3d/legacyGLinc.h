
#if defined(WIN32)
/* MS gl.h needs windows.h to be included first */
#include <windows.h>
#endif

#if defined(USE_APPLE_OPENGL)
#include <gl.h>
#include <glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#if defined(HAVE_GL_GLX_H)
#include <GL/glx.h>             /* x-windows file */
#endif
